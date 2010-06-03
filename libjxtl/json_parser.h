#ifndef JSON_PARSER_H
#define JSON_PARSER_H

#include "json_writer.h"

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

extern int json_file_parse( const char *json_file, json_writer_t *writer );

#endif
