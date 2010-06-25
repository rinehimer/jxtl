#ifndef JSON_PATH_H
#define JSON_PATH_H

#include <apr_pools.h>

#include "json_path_parse.h"
#include "json_path_lex.h"

#include "lex_extra.h"
#include "json_node.h"

/*
 * Parser callback prototypes.
 */
typedef struct json_path_callback_t {
  void ( *identifier_handler )( void *user_data, unsigned char *ident );
  void ( *root_object_handler )( void *user_data );
  void ( *current_object_handler )( void *user_data );
  void ( *all_children_handler )( void *user_data );
  void ( *test_start_handler )( void *user_data );
  void ( *test_end_handler )( void *user_data );
  void ( *negate_handler )( void *user_data );
  void *user_data;
} json_path_callback_t;

typedef enum json_path_expr_type {
  JSON_PATH_ROOT_OBJ,
  JSON_PATH_CURRENT_OBJ,
  JSON_PATH_ALL_CHILDREN,
  JSON_PATH_LOOKUP
}json_path_expr_type;

typedef struct json_path_expr_t {
  /** What type of expression this is. */
  json_path_expr_type type;
  /** A name to lookup. */
  unsigned char *identifier;
  /** The beginning of this expression. */
  struct json_path_expr_t *root;
  /** Next expression. */
  struct json_path_expr_t *next;
  /** A test to evaluate after lookup. */
  struct json_path_expr_t *test;
}json_path_expr_t;


typedef struct jsp_data {
  apr_pool_t *mp;
  /** Array to store the current expression.  */
  apr_array_header_t *expr_array;
  /** Root of the path expression. */
  json_path_expr_t *root;
  /** The current expression. */ 
  json_path_expr_t *curr;
}jsp_data;

typedef struct json_path_builder_t {
  apr_pool_t *mp;
  json_path_callback_t callbacks;
  yyscan_t scanner;
  lex_extra_t lex_extra;
  jsp_data data;
}json_path_builder_t;

typedef struct json_path_obj_t {
  apr_pool_t *mp;
  json_path_expr_t *expr;
  apr_array_header_t *nodes;
}json_path_obj_t;

extern json_path_expr_t *json_path_compile( json_path_builder_t *path_builder,
                                            const char *path );
extern int json_path_evaluate( const char *path,
                               json_t *json,
                               json_path_obj_t *path_ctx );
extern int json_path_compiled_eval( json_path_expr_t *expr,
                                    json_t *json,
                                    json_path_obj_t *obj );
#endif
