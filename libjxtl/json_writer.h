/*
 * json_writer.h
 *
 * Description
 *   The API for a JSON writer used to construct an in-memory tree.
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

#ifndef JSON_WRITER_H
#define JSON_WRITER_H

#include <apr_pools.h>
#include <apr_tables.h>

#include "json.h"
#include "json_writer_ctx.h"

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
