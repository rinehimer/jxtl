#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include <apr_pools.h>

#include "json.h"
#include "parser.h"

typedef struct json_callback_t {
  void ( *object_start_handler )( void *user_data );
  void ( *object_end_handler )( void *user_data );
  void ( *array_start_handler )( void *user_data );
  void ( *array_end_handler )( void *user_data );
  void ( *property_start_handler )( void *user_data, unsigned char *name );
  void ( *property_end_handler )( void *user_data );
  void ( *string_handler )( void *user_data, unsigned char *value );
  void ( *integer_handler )( void *user_data, int value );
  void ( *number_handler )( void *user_data, double value );
  void ( *boolean_handler )( void *user_data, int value );
  void ( *null_handler )( void *user_data );
  void *user_data;
} json_callback_t;

parser_t *json_parser_create( apr_pool_t *mp );
int json_parser_parse_file( parser_t *parser, const char *file, json_t **obj );

#endif
