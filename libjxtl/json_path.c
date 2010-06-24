#include <stdlib.h>

#include <apr_pools.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "json_node.h"
#include "json_path.h"
#include "json_path_parse.h"
#include "json_path_lex.h"
#include "lex_extra.h"

/*
 * Data used while parsing
 */
typedef struct jsp_data {
  apr_pool_t *mp;
  /** Array to store the current expression.  */
  apr_array_header_t *expr_array;
  /** Root of the path expression. */
  json_path_expr_t *root;
  /** The current expression. */ 
  json_path_expr_t *curr;
}jsp_data;

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

static json_path_expr_t *json_path_expr_create( jsp_data *data,
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

  /* Save off the current expression. */
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
  expr->test = data->root;

  data->curr = expr;
  data->root = expr->root;
}

/**
 * Negate the current expression.
 */
void json_path_negate( void *user_data )
{
  jsp_data *data = (jsp_data *) user_data;
}

/*****************************************************************************
 * End of parser callback functions.
 *****************************************************************************/

/**
 * Compile a JSON Path expression.
 */
json_path_expr_t *json_path_compile( const char *path )
{
  yyscan_t scanner;
  lex_extra_t lex_extra;
  YY_BUFFER_STATE buffer_state;
  int parse_result;
  char *eval_str;
  int eval_str_len;
  jsp_data data;


  /**
   * Fix this -- this function should should probably take a json_path_t or
   * something that contains a pool.
   */
  apr_pool_create( &data.mp, NULL );
  data.expr_array = apr_array_make( data.mp, 32,
                                    sizeof( json_path_expr_t * ) );
  data.root = NULL;
  data.curr = NULL;

  json_path_callback_t callbacks = {
    json_path_identifier,
    json_path_root_object,
    json_path_current_object,
    json_path_all_children,
    json_path_test_start,
    json_path_test_end,
    json_path_negate,
    &data
  };

  /*
   * Set up eval_str for flex.  Flex requires the last two bytes of a string
   * passed to yy_scan_buffer be the null terminator.
   */
  eval_str_len = strlen( path ) + 2;
  eval_str = malloc( eval_str_len );
  apr_cpystrn( eval_str, path, eval_str_len - 1 );
  eval_str[eval_str_len - 1] = '\0';

  json_path_lex_init( &scanner );
  buffer_state = json_path__scan_buffer( eval_str, eval_str_len, scanner );
  lex_extra_init( &lex_extra, NULL );
  json_path_set_extra( &lex_extra, scanner );
  parse_result = json_path_parse( scanner, &callbacks );
  lex_extra_destroy( &lex_extra );
  json_path_lex_destroy( scanner );
  
  free( eval_str );

  return data.root;
}

static void json_path_evaluate_internal( json_path_expr_t *expr,
                                         json_t *json,
                                         apr_array_header_t *nodes )
{
  int i;
  json_t *json_value = NULL;
  int test_result = 1;

  if ( !json )
    return;

  /*
   * We have an array, just iterate over all items automatically.
   */
  if ( json->type == JSON_ARRAY ) {
    for ( i = 0; i < json->value.array->nelts; i++ ) {
      json_value = APR_ARRAY_IDX( json->value.array, i, json_t * );
      json_path_evaluate_internal( expr, json_value, nodes );
    }
    return;
  }

  switch ( expr->type ) {
  case JSON_PATH_ROOT_OBJ:
    break;
    
  case JSON_PATH_CURRENT_OBJ:
    break;
    
  case JSON_PATH_ALL_CHILDREN:
    break;
    
  case JSON_PATH_LOOKUP:
    if ( json && json->type == JSON_OBJECT ) {
      json_value = apr_hash_get( json->value.object, expr->identifier,
                                 APR_HASH_KEY_STRING );
    }
    break;
  default:
    break;
  }

  if ( json_value && expr->test ) {
    apr_pool_t *mp;
    apr_pool_create( &mp, NULL );
    apr_array_header_t *test_nodes = apr_array_make( mp, 128,
                                                     sizeof(json_t *) );
    json_path_evaluate_internal( expr->test, json_value, test_nodes );
    test_result = test_nodes->nelts > 0;
    apr_pool_destroy( mp );
  }

  if ( json_value && test_result == 1 && !expr->next ) {
    /*
     * Looked something up, test was successful (or no test) and its the end
     * of the expression.
     */
    APR_ARRAY_PUSH( nodes, json_t * ) = json_value;
  }
  else if ( json_value && test_result == 1 && expr->next ) {
    /*
     * Looked something up, test was successful (or no test) and its not
     * the end of the expression.
     */
    json_path_evaluate_internal( expr->next, json_value, nodes );
  }

}

/**
 * Evaluate the given path expression in the context of json.
 * Returns an array of nodes.
 */
apr_array_header_t *json_path_evaluate( const char *path, json_t *json )
{
  json_path_expr_t *expr = json_path_compile( path );
  apr_array_header_t *nodes;
  apr_pool_t *mp;
  apr_pool_create( &mp, NULL );

  nodes = apr_array_make( mp, 128, sizeof(json_t *) );
  
  json_path_evaluate_internal( expr, json, nodes );

  int i;
  json_t *tmp_json;
  for ( i = 0; i < nodes->nelts; i++ ) {
    tmp_json = APR_ARRAY_IDX( nodes, i, json_t * );
    fprintf( stderr, "here is an object\n" );
    fprintf( stderr, "--------------------------------\n" );
    json_object_print( tmp_json, 1 );
    fprintf( stderr, "--------------------------------\n" );
  }

  return nodes;
}
