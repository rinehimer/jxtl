#ifndef JXTL_PATH_H
#define JXTL_PATH_H

#include <apr_tables.h>
#include <apr_pools.h>

#include "parser.h"
#include "json.h"
#include "jxtl_path_expr.h"

/*
 * Parser callback prototypes.
 */
typedef struct jxtl_path_callback_t {
  void ( *identifier_handler )( void *user_data, char *ident );
  void ( *root_object_handler )( void *user_data );
  void ( *parent_object_handler )( void *user_data );
  void ( *current_object_handler )( void *user_data );
  void ( *any_object_handler )( void *user_data );
  void ( *predicate_start_handler )( void *user_data );
  void ( *predicate_end_handler )( void *user_data );
  void ( *negate_handler )( void *user_data );
  void *user_data;
} jxtl_path_callback_t;

typedef struct jxtl_path_obj_t {
  apr_pool_t *mp;
  apr_array_header_t *nodes;
} jxtl_path_obj_t;

parser_t *jxtl_path_parser_create( apr_pool_t *mp );
int jxtl_path_parser_parse_buffer( parser_t *parser,
                                   const char *path,
                                   jxtl_path_callback_t *callbacks );

int jxtl_path_eval( apr_pool_t *mp, const char *path, json_t *json,
                    jxtl_path_obj_t **obj_ptr );
int jxtl_path_compiled_eval( apr_pool_t *mp, jxtl_path_expr_t *expr,
                             json_t *json, jxtl_path_obj_t **obj_ptr );

#endif
