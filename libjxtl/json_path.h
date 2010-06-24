#ifndef JSON_PATH_H
#define JSON_PATH_H

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

extern json_path_expr_t *json_path_compile( const char *json_file );
extern apr_array_header_t *json_path_evaluate( const char *path,
                                               json_t *json );
#endif
