/*
 * json.h
 *
 * Description
 *   Defines the API for parsing/creating/dumping JSON.
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

#ifndef JSON_H
#define JSON_H

#include <apr_general.h>
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_tables.h>

#include "parser.h"

/**
 * JSON types
 */
typedef enum json_type {
  JSON_STRING,
  JSON_INTEGER,
  JSON_NUMBER,
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_BOOLEAN,
  JSON_NULL
} json_type;

typedef struct json_t {
  char *name;
  json_type type;
  struct json_t *parent;
  union {
    char *string;
    int integer;
    double number;
    apr_hash_t *object;
    apr_array_header_t *array;
    int boolean;
  } value;
} json_t;

#define JSON_NAME( json ) (json)->name
#define JSON_IS_TYPE( json, json_type ) ( (json)->type == json_type )

#define JSON_IS_STRING( json ) JSON_IS_TYPE( json, JSON_STRING )
#define JSON_IS_INTEGER( json ) JSON_IS_TYPE( json, JSON_INTEGER )
#define JSON_IS_NUMBER( json ) JSON_IS_TYPE( json, JSON_NUMBER )
#define JSON_IS_OBJECT( json ) JSON_IS_TYPE( json, JSON_OBJECT )
#define JSON_IS_ARRAY( json ) JSON_IS_TYPE( json, JSON_ARRAY )
#define JSON_IS_BOOLEAN( json ) JSON_IS_TYPE( json, JSON_BOOLEAN )
#define JSON_IS_NULL( json ) JSON_IS_NULL( json, JSON_NULL )

#define JSON_IS_TRUE_BOOLEAN( json )\
 ( JSON_IS_BOOLEAN( json ) && (json)->value.boolean == TRUE )

json_t *json_create_str( apr_pool_t *mp, const char *string );
json_t *json_create_strn( apr_pool_t *mp, const char *string, int len );
json_t *json_create_integer( apr_pool_t *mp, int integer );
json_t *json_create_number( apr_pool_t *mp, double number );
json_t *json_create_object( apr_pool_t *mp );
json_t *json_create_array( apr_pool_t *mp );
json_t *json_create_boolean( apr_pool_t *mp, int boolean );
json_t *json_create_null( apr_pool_t *mp );

void json_dump( json_t *node, int indent );
void json_to_xml( json_t *json, int indent );

/**
 * Return the string value of a json object, or NULL if it type JSON_NULL,
 * JSON_OBJECT or JSON_ARRAY.
 */
char *json_get_string_value( apr_pool_t *mp, json_t *json );

typedef struct json_callback_t {
  void ( *object_start_handler )( void *user_data );
  void ( *object_end_handler )( void *user_data );
  void ( *array_start_handler )( void *user_data );
  void ( *array_end_handler )( void *user_data );
  void ( *property_start_handler )( void *user_data, const char *name );
  void ( *property_end_handler )( void *user_data );
  void ( *string_handler )( void *user_data, const char *value );
  void ( *integer_handler )( void *user_data, int value );
  void ( *number_handler )( void *user_data, double value );
  void ( *boolean_handler )( void *user_data, int value );
  void ( *null_handler )( void *user_data );
  void *user_data;
} json_callback_t;

parser_t *json_parser_create( apr_pool_t *mp );
int json_parser_parse_file( parser_t *parser, const void *file,
                            json_callback_t *json_callbacks );
int json_parser_parse_buffer( parser_t *parser, const void *buffer,
                              json_callback_t *json_callbacks );

int json_parser_parse_file_to_obj( apr_pool_t *mp, parser_t *parser,
                                   apr_file_t *file, json_t **obj );
int json_parser_parse_buffer_to_obj( apr_pool_t *mp, parser_t *parser,
                                     const char *buffer, json_t **obj );


#endif
