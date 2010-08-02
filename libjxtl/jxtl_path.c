#include <stdlib.h>

#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "json.h"
#include "jxtl_path.h"
#include "jxtl_path_parse.h"
#include "jxtl_path_lex.h"
#include "lex_extra.h"

#define NODELIST_SIZE 1024

jxtl_path_obj_t *jxtl_path_obj_create( apr_pool_t *mp )
{
  jxtl_path_obj_t *path_obj;

  if ( mp ) {
    path_obj = apr_palloc( mp, sizeof(jxtl_path_obj_t) );
    path_obj->free_func = NULL;
  }
  else {
    path_obj = malloc( sizeof(jxtl_path_obj_t) );
    path_obj->free_func = free;
  }

  apr_pool_create( &path_obj->mp, NULL );
  path_obj->nodes = NULL;
  return path_obj;
}

void jxtl_path_obj_destroy( jxtl_path_obj_t *path_obj )
{
  apr_pool_destroy( path_obj->mp );
  path_obj->nodes = NULL;

  if ( path_obj->free_func ) {
    path_obj->free_func( path_obj );
  }
}

static void expr_add( jsp_data *data, jxtl_path_expr_t *expr )
{
  if ( !data->root ) {
    data->root = expr;
  }

  if ( !data->curr ) {
    data->curr = expr;
  }
  else {
    data->curr->next = expr;
    data->curr = expr;
  }
}

static void jxtl_path_expr_create( jsp_data *data,
				   jxtl_path_expr_type type,
				   unsigned char *identifier )
{
  jxtl_path_expr_t *expr;

  expr = apr_palloc( data->mp, sizeof( jxtl_path_expr_t ) );
  expr->type = type;
  expr->identifier = identifier;
  expr->root = ( data->root ) ? data->root : expr;
  expr->next = NULL;
  expr->test = NULL;
  expr->negate = 0;
  expr_add( data, expr );
}

/*****************************************************************************
 * Parser callbacks
 *****************************************************************************/

/**
 * Request to lookup an identifier.
 */
void jxtl_path_identifier( void *user_data, unsigned char *ident )
{
  jsp_data *data = (jsp_data *) user_data;
  jxtl_path_expr_create( data, JXTL_PATH_LOOKUP,
                         (unsigned char *) apr_pstrdup( data->mp,
                                                        (char *) ident ) );
}

/**
 * Request for the root object.
 */
void jxtl_path_root_object( void *user_data )
{
  jxtl_path_expr_create( user_data, JXTL_PATH_ROOT_OBJ, NULL );
}

void jxtl_path_parent_object( void *user_data )
{
  jxtl_path_expr_create( user_data, JXTL_PATH_PARENT_OBJ, NULL );
}

/**
 * Request for the current object.
 */
void jxtl_path_current_object( void *user_data )
{
  jxtl_path_expr_create( user_data, JXTL_PATH_CURRENT_OBJ, NULL );

}

/**
 * Request for all children.
 */
void jxtl_path_all_children( void *user_data )
{
  jxtl_path_expr_create( user_data, JXTL_PATH_ALL_CHILDREN, NULL );
}

/**
 * Start a test.
 */
void jxtl_path_test_start( void *user_data )
{
  jsp_data *data = (jsp_data *) user_data;

  /*
   * Make sure we have started an expression, if not this is just a boolean
   * expression, create a place holder for it.
   */
  if ( !data->curr ) {
    jxtl_path_expr_create( user_data, JXTL_PATH_BOOLEAN_EXPR, NULL );
  }
  APR_ARRAY_PUSH( data->expr_array, jxtl_path_expr_t * ) = data->curr;

  data->root = NULL;
  data->curr = NULL;
}

/**
 * End a test.
 */
void jxtl_path_test_end( void *user_data )
{
  jsp_data *data = (jsp_data *) user_data;
  jxtl_path_expr_t *expr;
  expr = APR_ARRAY_POP( data->expr_array, jxtl_path_expr_t * );

  expr->test = data->root;
  data->curr = expr;
  data->root = expr->root;
}

/**
 * Negate the current expression.
 */
void jxtl_path_negate( void *user_data )
{
  jsp_data *data = (jsp_data *) user_data;
  data->root->negate = 1;
}

/*****************************************************************************
 * End of parser callback functions.
 *****************************************************************************/

void jxtl_path_builder_init( jxtl_path_builder_t *path_builder )
{
  apr_pool_create( &path_builder->mp, NULL );
  path_builder->data.expr_array = apr_array_make( path_builder->mp, 32,
                                                  sizeof(jxtl_path_expr_t *) );
  path_builder->data.mp = path_builder->mp;
  path_builder->data.root = NULL;
  path_builder->data.curr = NULL;

  path_builder->callbacks.identifier_handler = jxtl_path_identifier;
  path_builder->callbacks.root_object_handler = jxtl_path_root_object;
  path_builder->callbacks.parent_object_handler = jxtl_path_parent_object;
  path_builder->callbacks.current_object_handler = jxtl_path_current_object;
  path_builder->callbacks.all_children_handler = jxtl_path_all_children;
  path_builder->callbacks.test_start_handler = jxtl_path_test_start;
  path_builder->callbacks.test_end_handler = jxtl_path_test_end;
  path_builder->callbacks.negate_handler = jxtl_path_negate;
  path_builder->callbacks.user_data = &path_builder->data;

  jxtl_path_lex_init( &path_builder->scanner );
  lex_extra_init( &path_builder->lex_extra, NULL );
  jxtl_path_set_extra( &path_builder->lex_extra, path_builder->scanner );
}

void jxtl_path_builder_destroy( jxtl_path_builder_t *path_builder )
{
  lex_extra_destroy( &path_builder->lex_extra );
  jxtl_path_lex_destroy( path_builder->scanner );
  apr_pool_destroy( path_builder->mp );
}

/**
 * Compile a JSON Path expression.
 */
jxtl_path_expr_t *jxtl_path_compile( jxtl_path_builder_t *path_builder,
                                     const unsigned char *path )
{
  YY_BUFFER_STATE buffer_state;
  int parse_result;
  char *eval_str;
  int eval_str_len;

  APR_ARRAY_CLEAR( path_builder->data.expr_array );
  path_builder->data.root = NULL;
  path_builder->data.curr = NULL;

  /*
   * Set up eval_str for flex.  Flex requires the last two bytes of a string
   * passed to yy_scan_buffer be the null terminator.
   */
  eval_str_len = strlen( (char*) path ) + 2;
  eval_str = malloc( eval_str_len );
  apr_cpystrn( eval_str, (char *) path, eval_str_len - 1 );
  eval_str[eval_str_len - 1] = '\0';

  buffer_state = jxtl_path__scan_buffer( eval_str, eval_str_len,
                                         path_builder->scanner );
  parse_result = jxtl_path_parse( path_builder->scanner,
                                  &path_builder->callbacks );
  jxtl_path__delete_buffer( buffer_state, path_builder->scanner );
  
  free( eval_str );

  return path_builder->data.root;
}

static void jxtl_path_eval_internal( jxtl_path_expr_t *expr,
                                     json_t *json,
                                     apr_array_header_t *nodes );

static void jxtl_path_test_node( jxtl_path_expr_t *expr,
                                 json_t *json,
                                 apr_array_header_t *nodes )
{
  int test_result;

  if ( json ) {
    if ( expr && expr->test ) {
      /*
       * The expression has a a test.  If our current node is an array we need
       * to loop here and run the test on each node to see what should be
       * pushed or recursively evaluated.
       */
      apr_pool_t *mp;
      apr_pool_create( &mp, NULL );
      apr_array_header_t *test_nodes = apr_array_make( mp, NODELIST_SIZE,
                                                       sizeof(json_t *) );
      if ( json->type == JSON_ARRAY ) {
        int i;
        json_t *tmp_json;
        for ( i = 0; i < json->value.array->nelts; i++ ) {
          APR_ARRAY_CLEAR( test_nodes );
          tmp_json = APR_ARRAY_IDX( json->value.array, i, json_t * );
          jxtl_path_eval_internal( expr->test, tmp_json, test_nodes );
          test_result = test_nodes->nelts;
          test_result = ( expr->test->negate ) ? !test_result : test_result;
          if ( test_result && !expr->next ) {
            APR_ARRAY_PUSH( nodes, json_t * ) = tmp_json;
          }
          else if ( test_result && expr->next ) {
            jxtl_path_eval_internal( expr->next, tmp_json, nodes );
          }
        }
      }
      else {
        jxtl_path_eval_internal( expr->test, json, test_nodes );
        test_result = test_nodes->nelts > 0;
        test_result = ( expr->test->negate ) ? !test_result : test_result;
        if ( test_result && !expr->next ) {
          APR_ARRAY_PUSH( nodes, json_t * ) = json;
        }
        else if ( test_result && expr->next ) {
          jxtl_path_eval_internal( expr->next, json, nodes );
        }
      }
      apr_pool_destroy( mp );
    }
    else {
      if ( expr && expr->next ) {
        jxtl_path_eval_internal( expr->next, json, nodes );
      }
      else {
	if ( json->type == JSON_ARRAY ) {
	  int i;
	  json_t *tmp_json;
	  for ( i = 0; i < json->value.array->nelts; i++ ) {
	    tmp_json = APR_ARRAY_IDX( json->value.array, i, json_t * );
	    APR_ARRAY_PUSH( nodes, json_t * ) = tmp_json;
	  }
	}
	else {
	  APR_ARRAY_PUSH( nodes, json_t * ) = json;
	}
      }
    }
  }
}

static void jxtl_path_eval_internal( jxtl_path_expr_t *expr,
                                     json_t *json,
                                     apr_array_header_t *nodes )
{
  int i;
  json_t *tmp_json = NULL;
  apr_hash_index_t *idx;

  if ( !json )
    return;

  /*
   * We have an array, just iterate over all items.
   */
  if ( json->type == JSON_ARRAY ) {
    for ( i = 0; i < json->value.array->nelts; i++ ) {
      tmp_json = APR_ARRAY_IDX( json->value.array, i, json_t * );
      jxtl_path_eval_internal( expr, tmp_json, nodes );
    }
    return;
  }

  switch ( expr->type ) {
  case JXTL_PATH_BOOLEAN_EXPR:
    tmp_json = json;
    break;
    
  case JXTL_PATH_ROOT_OBJ:
    for( tmp_json = json; tmp_json && tmp_json->parent;
         tmp_json = tmp_json->parent );
    break;

  case JXTL_PATH_PARENT_OBJ:
    tmp_json = json->parent;
    if ( tmp_json && tmp_json->type == JSON_ARRAY ) {
      tmp_json = tmp_json->parent;
    }
    break;

  case JXTL_PATH_CURRENT_OBJ:
    tmp_json = json;
    break;
    
  case JXTL_PATH_ALL_CHILDREN:
    if ( json && json->type == JSON_OBJECT ) {
      for ( idx = apr_hash_first( NULL, json->value.object ); idx;
            idx = apr_hash_next( idx ) ) {
        apr_hash_this( idx, NULL, NULL, (void **) &tmp_json );
        jxtl_path_test_node( expr->next, tmp_json, nodes );
      }
    }
    return;
    break;
    
  case JXTL_PATH_LOOKUP:
    if ( json && json->type == JSON_OBJECT ) {
      tmp_json = apr_hash_get( json->value.object, expr->identifier,
                               APR_HASH_KEY_STRING );
    }
    break;

  default:
    break;
  }

  jxtl_path_test_node( expr, tmp_json, nodes );
}

/**
 * Evaluate the given path expression in the context of json.
 * Returns the number of nodes selected.
 */
int jxtl_path_eval( const unsigned char *path, json_t *json,
		    jxtl_path_obj_t *obj )
{
  jxtl_path_builder_t path_builder;
  jxtl_path_expr_t *expr;

  apr_pool_clear( obj->mp );
  obj->nodes = apr_array_make( obj->mp, NODELIST_SIZE, sizeof(json_t *) );

  jxtl_path_builder_init( &path_builder );
  expr = jxtl_path_compile( &path_builder, path );
  jxtl_path_eval_internal( expr, json, obj->nodes );
  jxtl_path_builder_destroy( &path_builder );

  return obj->nodes->nelts;
}

/**
 * Evaluate a pre-compiled expression.  Returns the number of nodes.
 */
int jxtl_path_compiled_eval( jxtl_path_expr_t *expr,
                             json_t *json,
                             jxtl_path_obj_t *obj )
{
  apr_pool_clear( obj->mp );
  obj->nodes = apr_array_make( obj->mp, NODELIST_SIZE, sizeof(json_t *) );
  jxtl_path_eval_internal( expr, json, obj->nodes );
  return obj->nodes->nelts;
}
