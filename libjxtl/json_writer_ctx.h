/*
 * $Id$
 *
 * Description
 *   The API for a JSON writer context.  This is used by the JSON writer to
 *   validate that an action is valid.
 *
 * Copyright 2010 Dan Rinehimer
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JSON_WRITER_CTX_H
#define JSON_WRITER_CTX_H

#include <apr_pools.h>
#include <apr_tables.h>

typedef enum json_writer_ctx_state {
  JSON_INITIAL,
  JSON_IN_OBJECT,
  JSON_IN_ARRAY,
  JSON_PROPERTY
} json_writer_ctx_state;

typedef struct json_writer_ctx_t {
  apr_pool_t *mp;
  int depth;
  apr_array_header_t *prop_stack;
  apr_array_header_t *state_stack;
} json_writer_ctx_t;

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
json_writer_ctx_state json_writer_ctx_get_state( json_writer_ctx_t *context );

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

#endif
