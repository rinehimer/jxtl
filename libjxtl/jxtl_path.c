#include <stdlib.h>

#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "json.h"
#include "jxtl_path.h"
#include "jxtl_path_parse.h"
#include "jxtl_path_lex.h"
#include "parser.h"

#define NODELIST_SIZE 1024

static void jxtl_path_eval_internal( jxtl_path_expr_t *expr,
                                     json_t *json,
                                     apr_array_header_t *nodes,
                                     int predicate_depth );
static void jxtl_path_test_node( jxtl_path_expr_t *expr,
                                 json_t *json,
                                 apr_array_header_t *nodes,
                                 int predicate_depth );

static jxtl_path_obj_t *jxtl_path_obj_create( apr_pool_t *mp )
{
  jxtl_path_obj_t *path_obj;

  path_obj = apr_palloc( mp, sizeof(jxtl_path_obj_t) );
  path_obj->mp = mp;
  path_obj->nodes = apr_array_make( mp, NODELIST_SIZE, sizeof(json_t *) );

  return path_obj;
}

/**
 * Finish a predicate.  If it evaluated to true and the expression is done,
 * push the node on the result stack.  If it evaluated to true and the
 * expression is not done, recursively evaluate.  If it's false, do nothing.
 */
static void jxtl_finish_predicate( jxtl_path_expr_t *expr,
                                   json_t *json,
                                   apr_array_header_t *nodes,
                                   int predicate_nodes,
                                   int predicate_depth )
{
  int result;
  result = ( expr->predicate->negate ) ? !predicate_nodes : predicate_nodes;
  if ( result && !expr->next ) {
    APR_ARRAY_PUSH( nodes, json_t * ) = json;
  }
  else if ( result && expr->next ) {
    jxtl_path_eval_internal( expr->next, json, nodes, predicate_depth );
  }
}

static void jxtl_path_test_node( jxtl_path_expr_t *expr,
                                 json_t *json,
                                 apr_array_header_t *nodes,
                                 int predicate_depth )
{
  if ( json && expr ) {
    if ( expr->predicate ) {
      /*
       * The expression has a a predicate.  If our current node is an array we
       * need to loop here and evaluate the predicate on each node to see what
       * should be pushed.  Otherwise, we just recursively evaluate it.
       */
      apr_pool_t *mp;
      apr_pool_create( &mp, NULL );
      apr_array_header_t *predicate_nodes = apr_array_make( mp, NODELIST_SIZE,
                                                            sizeof(json_t *) );
      if ( json->type == JSON_ARRAY ) {
        int i;
        json_t *tmp_json;
        for ( i = 0; i < json->value.array->nelts; i++ ) {
          APR_ARRAY_CLEAR( predicate_nodes );
          tmp_json = APR_ARRAY_IDX( json->value.array, i, json_t * );
          jxtl_path_eval_internal( expr->predicate, tmp_json, predicate_nodes,
                                   predicate_depth + 1 );
          jxtl_finish_predicate( expr, tmp_json, nodes,
                                 predicate_nodes->nelts, predicate_depth );
        }
      }
      else {
        jxtl_path_eval_internal( expr->predicate, json, predicate_nodes,
                                 predicate_depth + 1 );
        jxtl_finish_predicate( expr, json, nodes, predicate_nodes->nelts,
                               predicate_depth );
      }
      apr_pool_destroy( mp );
    }
    else if ( expr->next ) {
      /* No predicate, but expression keeps going. */
      jxtl_path_eval_internal( expr->next, json, nodes, predicate_depth );
    }
    else {
      /* This is the end of the expression, push on whatever nodes are left. */
      if ( json->type == JSON_ARRAY ) {
        int i;
        json_t *tmp_json;
        for ( i = 0; i < json->value.array->nelts; i++ ) {
          tmp_json = APR_ARRAY_IDX( json->value.array, i, json_t * );
          APR_ARRAY_PUSH( nodes, json_t * ) = tmp_json;
        }
      }
      else {
        if ( ( predicate_depth == 0 ) ||
             ( !JSON_IS_BOOLEAN( json ) ) ||
             ( JSON_IS_TRUE_BOOLEAN( json ) ) ) {
          APR_ARRAY_PUSH( nodes, json_t * ) = json;
        }
      }
    }
  }
}

static void jxtl_path_eval_internal( jxtl_path_expr_t *expr,
                                     json_t *json,
                                     apr_array_header_t *nodes,
                                     int predicate_depth )
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
      jxtl_path_eval_internal( expr, tmp_json, nodes, predicate_depth );
    }
    return;
  }

  switch ( expr->type ) {
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
    
  case JXTL_PATH_ANY_OBJ:
    if ( json && json->type == JSON_OBJECT ) {
      for ( idx = apr_hash_first( NULL, json->value.object ); idx;
            idx = apr_hash_next( idx ) ) {
        apr_hash_this( idx, NULL, NULL, (void **) &tmp_json );
        jxtl_path_test_node( expr, tmp_json, nodes, predicate_depth );
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

  jxtl_path_test_node( expr, tmp_json, nodes, predicate_depth );
}

/**
 * Evaluate a pre-compiled expression.  Returns the number of nodes.
 */
int jxtl_path_compiled_eval( apr_pool_t *mp, jxtl_path_expr_t *expr,
                             json_t *json, jxtl_path_obj_t **obj_ptr )
{
  jxtl_path_obj_t *obj;

  *obj_ptr = NULL;

  if ( !expr ) {
    return 0;
  }

  obj = jxtl_path_obj_create( mp );
  jxtl_path_eval_internal( expr, json, obj->nodes, 0 );
  *obj_ptr = obj;

  return obj->nodes->nelts;
}

/**
 * Evaluate the given path expression in the context of json.
 * Returns the number of nodes selected or -1 if there was an error parsing
 * the expression.
 */
int jxtl_path_eval( apr_pool_t *mp, const unsigned char *path, json_t *json,
                    jxtl_path_obj_t **obj_ptr )
{

  jxtl_path_expr_t *expr;
  parser_t *parser;

  parser = jxtl_path_parser_create( mp );

  if ( jxtl_path_parser_parse_buffer( parser, path, &expr ) ) {
    return jxtl_path_compiled_eval( mp, expr, json, obj_ptr );
  }
  else {
    return -1;
  }
}
