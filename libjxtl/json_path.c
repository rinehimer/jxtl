#include <stdlib.h>

#include <apr_pools.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "json_node.h"
#include "json_path.h"
#include "json_path_parse.h"
#include "json_path_lex.h"
#include "lex_extra.h"

void json_path_obj_init( json_path_obj_t *json_path_obj )
{
  apr_pool_create( &json_path_obj->mp, NULL );
  json_path_obj->expr = NULL;
  json_path_obj->nodes = apr_array_make( json_path_obj->mp, 128,
                                         sizeof(json_t *) );
}

void json_path_obj_destroy( json_path_obj_t *json_path_obj )
{
  apr_pool_destroy( json_path_obj->mp );
  json_path_obj->expr = NULL;
  json_path_obj->nodes = NULL;
}

static void expr_add( jsp_data *data, json_path_expr_t *expr )
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

static void json_path_expr_create( jsp_data *data,
				   json_path_expr_type type,
				   unsigned char *identifier )
{
  json_path_expr_t *expr;

  expr = apr_palloc( data->mp, sizeof( json_path_expr_t ) );
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
void json_path_identifier( void *user_data, unsigned char *ident )
{
  jsp_data *data = (jsp_data *) user_data;
  json_path_expr_create( data, JSON_PATH_LOOKUP,
                         (unsigned char *) apr_pstrdup( data->mp,
                                                        (char *) ident ) );
}

/**
 * Request for the root object.
 */
void json_path_root_object( void *user_data )
{
  json_path_expr_create( user_data, JSON_PATH_ROOT_OBJ, NULL );
}

void json_path_parent_object( void *user_data )
{
  json_path_expr_create( user_data, JSON_PATH_PARENT_OBJ, NULL );
}

/**
 * Request for the current object.
 */
void json_path_current_object( void *user_data )
{
  json_path_expr_create( user_data, JSON_PATH_CURRENT_OBJ, NULL );

}

/**
 * Request for all children.
 */
void json_path_all_children( void *user_data )
{
  json_path_expr_create( user_data, JSON_PATH_ALL_CHILDREN, NULL );
}

/**
 * Start a test.
 */
void json_path_test_start( void *user_data )
{
  jsp_data *data = (jsp_data *) user_data;

  /*
   * Make sure we have started an expression, if not this is just a boolean
   * expression, create a place holder for it.
   */
  if ( !data->curr ) {
    json_path_expr_create( user_data, JSON_PATH_BOOLEAN_EXPR, NULL );
  }
  APR_ARRAY_PUSH( data->expr_array, json_path_expr_t * ) = data->curr;

  data->root = NULL;
  data->curr = NULL;
}

/**
 * End a test.
 */
void json_path_test_end( void *user_data )
{
  jsp_data *data = (jsp_data *) user_data;
  json_path_expr_t *expr;
  expr = APR_ARRAY_POP( data->expr_array, json_path_expr_t * );

  /*
   * The root expression could be a test, i.e. '(a.b)'.
   */
  if ( expr ) {
    expr->test = data->root;
    data->curr = expr;
    data->root = expr->root;
  }
}

/**
 * Negate the current expression.
 */
void json_path_negate( void *user_data )
{
  jsp_data *data = (jsp_data *) user_data;
  data->root->negate = 1;
}

/*****************************************************************************
 * End of parser callback functions.
 *****************************************************************************/

void json_path_builder_init( json_path_builder_t *path_builder )
{
  apr_pool_create( &path_builder->mp, NULL );
  path_builder->data.expr_array = apr_array_make( path_builder->mp, 32,
                                                  sizeof(json_path_expr_t *) );
  path_builder->data.mp = path_builder->mp;
  path_builder->data.root = NULL;
  path_builder->data.curr = NULL;

  path_builder->callbacks.identifier_handler = json_path_identifier;
  path_builder->callbacks.root_object_handler = json_path_root_object;
  path_builder->callbacks.parent_object_handler = json_path_parent_object;
  path_builder->callbacks.current_object_handler = json_path_current_object;
  path_builder->callbacks.all_children_handler = json_path_all_children;
  path_builder->callbacks.test_start_handler = json_path_test_start;
  path_builder->callbacks.test_end_handler = json_path_test_end;
  path_builder->callbacks.negate_handler = json_path_negate;
  path_builder->callbacks.user_data = &path_builder->data;

  json_path_lex_init( &path_builder->scanner );
  lex_extra_init( &path_builder->lex_extra, NULL );
  json_path_set_extra( &path_builder->lex_extra, path_builder->scanner );
}

void json_path_builder_destroy( json_path_builder_t *path_builder )
{
  lex_extra_destroy( &path_builder->lex_extra );
  json_path_lex_destroy( path_builder->scanner );
  apr_pool_destroy( path_builder->mp );
}

/**
 * Compile a JSON Path expression.
 */
json_path_expr_t *json_path_compile( json_path_builder_t *path_builder,
                                     const unsigned char *path )
{
  YY_BUFFER_STATE buffer_state;
  int parse_result;
  char *eval_str;
  int eval_str_len;
  jsp_data data;

  APR_ARRAY_CLEAR( path_builder->data.expr_array );
  path_builder->data.root = NULL;
  path_builder->data.curr = NULL;

  /*
   * Set up eval_str for flex.  Flex requires the last two bytes of a string
   * passed to yy_scan_buffer be the null terminator.
   */
  eval_str_len = strlen( path ) + 2;
  eval_str = malloc( eval_str_len );
  apr_cpystrn( eval_str, path, eval_str_len - 1 );
  eval_str[eval_str_len - 1] = '\0';

  buffer_state = json_path__scan_buffer( eval_str, eval_str_len,
                                         path_builder->scanner );
  parse_result = json_path_parse( path_builder->scanner,
                                  &path_builder->callbacks );
  json_path__delete_buffer( buffer_state, path_builder->scanner );
  
  free( eval_str );

  return path_builder->data.root;
}

static void json_path_eval_internal( json_path_expr_t *expr,
                                     json_t *json,
                                     apr_array_header_t *nodes );

static void json_path_test_node( json_path_expr_t *expr,
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
      apr_array_header_t *test_nodes = apr_array_make( mp, 128,
                                                       sizeof(json_t *) );
      if ( json->type == JSON_ARRAY ) {
        int i;
        json_t *tmp_json;
        for ( i = 0; i < json->value.array->nelts; i++ ) {
          APR_ARRAY_CLEAR( test_nodes );
          tmp_json = APR_ARRAY_IDX( json->value.array, i, json_t * );
          json_path_eval_internal( expr->test, tmp_json, test_nodes );
          test_result = test_nodes->nelts;
          test_result = ( expr->test->negate ) ? !test_result : test_result;
          if ( test_result && !expr->next ) {
            APR_ARRAY_PUSH( nodes, json_t * ) = tmp_json;
          }
          else if ( test_result && expr->next ) {
            json_path_eval_internal( expr->next, tmp_json, nodes );
          }
        }
      }
      else {
        json_path_eval_internal( expr->test, json, test_nodes );
        test_result = test_nodes->nelts > 0;
        test_result = ( expr->test->negate ) ? !test_result : test_result;
        if ( test_result && !expr->next ) {
          APR_ARRAY_PUSH( nodes, json_t * ) = json;
        }
        else if ( test_result && expr->next ) {
          json_path_eval_internal( expr->next, json, nodes );
        }
      }
      apr_pool_destroy( mp );
    }
    else {
      if ( expr && expr->next ) {
        json_path_eval_internal( expr->next, json, nodes );
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

static void json_path_eval_internal( json_path_expr_t *expr,
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
      json_path_eval_internal( expr, tmp_json, nodes );
    }
    return;
  }

  switch ( expr->type ) {
  case JSON_PATH_BOOLEAN_EXPR:
    tmp_json = json;
    break;
    
  case JSON_PATH_ROOT_OBJ:
    for( tmp_json = json; tmp_json && tmp_json->parent;
         tmp_json = tmp_json->parent );
    break;

  case JSON_PATH_PARENT_OBJ:
    tmp_json = json->parent;
    if ( tmp_json && tmp_json->type == JSON_ARRAY ) {
      tmp_json = tmp_json->parent;
    }
    break;

  case JSON_PATH_CURRENT_OBJ:
    tmp_json = json;
    break;
    
  case JSON_PATH_ALL_CHILDREN:
    if ( json && json->type == JSON_OBJECT ) {
      for ( idx = apr_hash_first( NULL, json->value.object ); idx;
            idx = apr_hash_next( idx ) ) {
        apr_hash_this( idx, NULL, NULL, (void **) &tmp_json );
        json_path_test_node( expr->next, tmp_json, nodes );
      }
    }
    return;
    break;
    
  case JSON_PATH_LOOKUP:
    if ( json && json->type == JSON_OBJECT ) {
      tmp_json = apr_hash_get( json->value.object, expr->identifier,
                               APR_HASH_KEY_STRING );
    }
    break;

  default:
    break;
  }

  json_path_test_node( expr, tmp_json, nodes );
}

/**
 * Evaluate the given path expression in the context of json.
 * Returns the number of nodes selected.
 */
int json_path_eval( const unsigned char *path, json_t *json,
		    json_path_obj_t *obj )
{
  json_path_builder_t path_builder;

  APR_ARRAY_CLEAR( obj->nodes );
  apr_pool_clear( obj->mp );

  json_path_builder_init( &path_builder );
  obj->expr = json_path_compile( &path_builder, path );
  json_path_eval_internal( obj->expr, json, obj->nodes );

  return obj->nodes->nelts;
}

/**
 * Evaluate a pre-compiled expression.  Returns the number of nodes.
 */
int json_path_compiled_eval( json_path_expr_t *expr,
                             json_t *json,
                             json_path_obj_t *obj )
{
  APR_ARRAY_CLEAR( obj->nodes );
  apr_pool_clear( obj->mp );
  json_path_eval_internal( expr, json, obj->nodes );
  return obj->nodes->nelts;
}
