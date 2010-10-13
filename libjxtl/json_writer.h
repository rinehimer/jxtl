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
  /**
   * Memory pool passed to the create function that is used to allocate the
   * writer and do internal allocations.
   */
  apr_pool_t *mp;

  /**
   * Memory pool for allocating the JSON objects.  Will be the same as the the
   * above memory pool if it is not passed.
   */
  apr_pool_t *json_mp;
  
  /**
   * Root node of the JSON created by the writer.
   */
  json_t *json;

  /**
   * A writer context to validate actions.
   */
  json_writer_ctx_t *context;

  /**
   * A stack of arrays and objects for building the JSON.
   */
  apr_array_header_t *json_stack;
} json_writer_t;

/**
 * Create a new writer context.
 * @param mp Pool to allocate the writer out of.
 * @return The writer context created.
 */
json_writer_ctx_t *json_writer_ctx_create( apr_pool_t *mp );

/**
 * Get the current state of the writer context.
 * @param The writer context.
 * @return The current state.
 */
json_state json_writer_ctx_get_state( json_writer_ctx_t *context );

/**
 * Determine whether or not starting and object is allowed in the current
 * context.
 * @param The writer context.
 * @return TRUE if we can start an object or array, FALSE otherwise.
 */
int json_writer_ctx_can_start_object_or_array( json_writer_ctx_t *context );

/**
 * Get the name of the current property.
 * @param The writer context.
 * @return The current property.
 */
unsigned char *json_writer_ctx_get_prop( json_writer_ctx_t *context );

/**
 * Attempt to start and object.
 * @param The writer context.
 * @return TRUE if the object was started, FALSE if it could not be started.
 */
int json_writer_ctx_start_object( json_writer_ctx_t *context );

/**
 * Attempt to end an object.
 * @param The writer context.
 * @return TRUE if the object was ended, FALSE if it could not be ended.
 */
int json_writer_ctx_end_object( json_writer_ctx_t *context );

/**
 * Attempt to start an array.
 * @param The writer context.
 * @return TRUE if the array was started, FALSE if it could not be started.
 */
int json_writer_ctx_start_array( json_writer_ctx_t *context );

/**
 * Attempt to end an array.
 * @param The writer context.
 * @return TRUE if the array was ended, FALSE if it could not be ended.
 */
int json_writer_ctx_end_array( json_writer_ctx_t *context );

/**
 * Start a property.
 * @param The writer context.
 * @return TRUE if the property was stated, FALSE if it could not be started.
 */
int json_writer_ctx_start_property( json_writer_ctx_t *context,
				    unsigned char *name );

/**
 * End a property.
 * @param The writer context.
 * @return Always returns TRUE right now.
 */
int json_writer_ctx_end_property( json_writer_ctx_t *context );

/**
 * Test whether or not a simple value can be written.
 * @param The writer context.
 * @return TRUE if a simple value can be written, FALSE otherwise.
 */
int json_writer_ctx_can_write_value( json_writer_ctx_t *context );

/**
 * Create a new JSON writer.
 * @param mp Pool to allocate the writer out of.
 * @param json_mp Pool to use for allocating the JSON object.  If NULL, the
 *        same as mp.
 * @return The writer created.
 */
json_writer_t *json_writer_create( apr_pool_t *mp, apr_pool_t *json_mp );

/**
 * Start an object.
 * @param writer_ptr The JSON writer.
 */
void json_writer_start_object( void *writer_ptr );

/**
 * End an object.
 * @param writer_ptr The JSON writer.
 */
void json_writer_end_object( void *writer_ptr );

/**
 * Start an array.
 * @param writer_ptr The JSON writer.
 */
void json_writer_start_array( void *writer_ptr );

/**
 * End an array.
 * @param writer_ptr The JSON writer.
 */
void json_writer_end_array( void *writer_ptr );

/**
 * Start a property.
 * @param writer_ptr The JSON writer.
 * @param name The name of the property to start.
 */
void json_writer_start_property( void *writer_ptr, unsigned char *name );

/**
 * End a property.
 * @param writer_ptr The JSON writer.
 */
void json_writer_end_property( void *writer_ptr );

/**
 * Write a string.
 * @param writer_ptr The JSON writer.
 * @param value The string to write.
 */
void json_writer_write_string( void *writer_ptr, unsigned char *value );

/**
 * Write an integer.
 * @param writer_ptr The JSON writer.
 * @param value The integer to write.
 */
void json_writer_write_integer( void *writer_ptr, int value );

/**
 * Write a real number.
 * @param writer_ptr The JSON writer.
 * @param value The number to write.
 */
void json_writer_write_number( void *writer_ptr, double value );

/**
 * Write a boolean.
 * @param writer_ptr The JSON writer.
 * @param value The boolean to write.
 */
void json_writer_write_boolean( void *writer_ptr, int value );

/**
 * Write a null value.
 * @param writer_ptr The JSON writer.
 */
void json_writer_write_null( void *writer_ptr );

#endif
