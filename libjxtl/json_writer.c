/*
 * json_writer.c
 *
 * Description
 *   Implementation of the JSON writer.
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

#include <stdio.h>
#include <apr_general.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "json.h"
#include "json_writer_ctx.h"
#include "json_writer.h"

json_writer_t *json_writer_create( apr_pool_t *mp, apr_pool_t *json_mp )
{
  json_writer_t *writer;
  writer = apr_palloc( mp, sizeof( json_writer_t ) );
  writer->mp = mp;
  writer->json_mp = ( json_mp ) ? json_mp : mp;
  writer->context = json_writer_ctx_create( writer->mp );
  writer->json = NULL;
  writer->json_stack = apr_array_make( writer->mp, 1024, sizeof( json_t * ) );
  return writer;
}

static void json_writer_error( const char *error_string, ... )
{
  va_list args;
  fprintf( stderr, "json_writer error:  " );
  va_start( args, error_string );
  vfprintf( stderr, error_string, args );
  fprintf( stderr, "\n" );
  va_end( args );
}

static void json_add( json_writer_t *writer, json_t *json )
{
  json_t *obj = NULL;
  json_t *tmp_json;
  json_t *new_array;
  unsigned char *name;

  if ( writer->json_stack->nelts > 0 ) {
    obj = APR_ARRAY_TAIL( writer->json_stack, json_t * );
  }

  if ( !obj ) {
    writer->json = json;
    return;
  }

  switch ( obj->type ) {
  case JSON_OBJECT:
    name = apr_pstrdup( writer->json_mp,
                        json_writer_ctx_get_prop( writer->context ) );
    JSON_NAME( json ) = name;
    tmp_json = apr_hash_get( obj->value.object, JSON_NAME( json ),
                             APR_HASH_KEY_STRING );
    if ( tmp_json && tmp_json->type != JSON_ARRAY ) {
      /* Key already exists, make an array and put both objects in it. */
      new_array = json_create_array( writer->json_mp );
      JSON_NAME( new_array ) = JSON_NAME( json );
      JSON_NAME( json ) = NULL;
      JSON_NAME( tmp_json ) = NULL;
      json->parent = new_array;
      tmp_json->parent = new_array;
      new_array->parent = obj;
      APR_ARRAY_PUSH( new_array->value.array, json_t * ) = tmp_json;
      APR_ARRAY_PUSH( new_array->value.array, json_t * ) = json;
      apr_hash_set( obj->value.object, JSON_NAME( new_array ),
                    APR_HASH_KEY_STRING, new_array );
    }
    else if ( tmp_json && tmp_json->type == JSON_ARRAY ) {
      /* Exists, but we already converted it to an array */
      json->parent = tmp_json;
      json->name = NULL;
      APR_ARRAY_PUSH( tmp_json->value.array, json_t * ) = json;
    }
    else {
      /* Standard insertion */
      json->parent = obj;
      apr_hash_set( obj->value.object, JSON_NAME( json ),
                    APR_HASH_KEY_STRING, json );
    }
    break;

  case JSON_ARRAY:
    json->parent = obj;
    APR_ARRAY_PUSH( obj->value.array, json_t * ) = json;
    break;

  default:
    json_writer_error( "values can only be added to arrays or objects" );
    break;
  }
}

/**
 * Start an object.
 * @param writer The json_writer object.
 */
void json_writer_start_object( void *writer_ptr )
{
  json_t *json;
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_start_object( writer->context ) ) {
    json_writer_error( "could not start object" );
    return;
  }

  json = json_create_object( writer->json_mp );
  json_add( writer, json );

  APR_ARRAY_PUSH( writer->json_stack, json_t * ) = json;
}

void json_writer_end_object( void *writer_ptr )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_end_object( writer->context ) ) {
    json_writer_error( "could not end object" );
    return;
  }

  apr_array_pop( writer->json_stack );
}

void json_writer_start_array( void *writer_ptr )
{
  json_t *json;
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_start_array( writer->context ) ) {
    json_writer_error( "could not start array" );
    return;
  }

  json = json_create_array( writer->json_mp );
  json_add( writer, json );

  APR_ARRAY_PUSH( writer->json_stack, json_t * ) = json;
}

void json_writer_end_array( void *writer_ptr )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_end_array( writer->context ) ) {
    json_writer_error( "could not end array" );
    return;
  }

  apr_array_pop( writer->json_stack );
}

/**
 * Save off a property.  Must be in an object for this to be valid.
 * @param writer The json_writer object.
 * @param name The name of the property to save.
 */
void json_writer_start_property( void *writer_ptr, unsigned char *name )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_start_property( writer->context, name ) )
    json_writer_error( "could not start property \"%s\"", name );
}

void json_writer_end_property( void *writer_ptr )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_end_property( writer->context ) )
    json_writer_error( "could not end property" );
}

void json_writer_write_string( void *writer_ptr, unsigned char *value )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_can_write_value( writer->context ) ) {
    json_writer_error( "could not write string \"%s\"", value );
    return;
  }

  json_add( writer, json_create_string( writer->json_mp, value ) );
}

void json_writer_write_integer( void *writer_ptr, int value )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_can_write_value( writer->context ) ) {
    json_writer_error( "could not write int \"%d\"", value );
    return;
  }

  json_add( writer, json_create_integer( writer->json_mp, value ) );
}

void json_writer_write_number( void *writer_ptr, double value )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_can_write_value( writer->context ) ) {
    json_writer_error( "could not write number \"%lf\"", value );
    return;
  }

  json_add( writer, json_create_number( writer->json_mp, value ) );
}

void json_writer_write_boolean( void *writer_ptr, int value )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_can_write_value( writer->context ) ) {
    json_writer_error( "could not write bool" );
    return;
  }

  json_add( writer, json_create_boolean( writer->json_mp, value ) );
}

void json_writer_write_null( void *writer_ptr )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_can_write_value( writer->context ) ) {
    json_writer_error( "could not write null" );
    return;
  }

  json_add( writer, json_create_null( writer->json_mp ) );
}
