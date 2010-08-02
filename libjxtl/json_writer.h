#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include <apr_pools.h>
#include <apr_tables.h>

#include "json.h"

typedef enum json_state {
  JSON_INITIAL,
  JSON_IN_OBJECT,
  JSON_IN_ARRAY,
  JSON_PROPERTY
} json_state;

typedef struct json_writer_ctx_t {
  apr_pool_t *mp;
  int depth;
  apr_array_header_t *prop_stack;
  apr_array_header_t *state_stack;
} json_writer_ctx_t;

typedef struct json_writer_t {
  apr_pool_t *mp;
  json_t *json;
  json_writer_ctx_t *context;
  apr_array_header_t *json_stack;
} json_writer_t;

void json_writer_object_start( void *writer_ptr );
void json_writer_object_end( void *writer_ptr );
void json_writer_array_start( void *writer_ptr );
void json_writer_array_end( void *writer_ptr );
void json_writer_property_start( void *writer_ptr, unsigned char *name );
void json_writer_property_end( void *writer_ptr );
void json_writer_string_write( void *writer_ptr, unsigned char *value );
void json_writer_integer_write( void *writer_ptr, int value );
void json_writer_number_write( void *writer_ptr, double value );
void json_writer_boolean_write( void *writer_ptr, int value );
void json_writer_null_write( void *writer_ptr );

#endif
