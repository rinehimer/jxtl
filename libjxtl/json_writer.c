#include <stdio.h>
#include <apr_general.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "json.h"
#include "json_writer.h"

json_writer_ctx_t *json_writer_ctx_create( apr_pool_t *mp )
{
  json_writer_ctx_t *context;
  context = apr_palloc( mp, sizeof( json_writer_ctx_t ) );
  context->mp = mp;
  context->depth = 0;
  context->prop_stack = apr_array_make( context->mp, 1024,
					sizeof( unsigned char * ) );
  context->state_stack = apr_array_make( context->mp, 1024,
					 sizeof( json_state ) );
  APR_ARRAY_PUSH( context->state_stack, json_state ) = JSON_INITIAL;
  return context;
}

json_state json_writer_ctx_get_state( json_writer_ctx_t *context )
{
  return APR_ARRAY_TAIL( context->state_stack,json_state );
}

int json_writer_ctx_can_start_object_or_array( json_writer_ctx_t *context )
{
  json_state state = json_writer_ctx_get_state( context );
  return ( state == JSON_INITIAL || state == JSON_PROPERTY ||
	   state == JSON_IN_ARRAY );
}

unsigned char *json_writer_ctx_get_prop( json_writer_ctx_t *context )
{
  return APR_ARRAY_TAIL( context->prop_stack, unsigned char * );
}

int json_writer_ctx_start_object( json_writer_ctx_t *context )
{
  if ( !json_writer_ctx_can_start_object_or_array( context ) )
    return FALSE;

  APR_ARRAY_PUSH( context->state_stack, json_state ) = JSON_IN_OBJECT;
  return TRUE;
}

int json_writer_ctx_end_object( json_writer_ctx_t *context )
{
  if ( json_writer_ctx_get_state( context ) != JSON_IN_OBJECT )
    return FALSE;

  apr_array_pop( context->state_stack );
  return TRUE;
}

int json_writer_ctx_start_array( json_writer_ctx_t *context )
{
  if ( !json_writer_ctx_can_start_object_or_array( context ) )
    return FALSE;

  APR_ARRAY_PUSH( context->state_stack, json_state ) = JSON_IN_ARRAY;
  return TRUE;
}

int json_writer_ctx_end_array( json_writer_ctx_t *context )
{
  if ( json_writer_ctx_get_state( context ) != JSON_IN_ARRAY )
    return FALSE;

  apr_array_pop( context->state_stack );
  return TRUE;
}

int json_writer_ctx_start_property( json_writer_ctx_t *context,
				    unsigned char *name )
{
  unsigned char *name_copy;
  if ( json_writer_ctx_get_state( context ) != JSON_IN_OBJECT )
    return FALSE;

  name_copy = (unsigned char *) apr_pstrdup( context->mp, (char *) name );

  APR_ARRAY_PUSH( context->prop_stack, unsigned char * ) = name_copy;
  APR_ARRAY_PUSH( context->state_stack, json_state ) = JSON_PROPERTY;
  return TRUE;
}

int json_writer_ctx_end_property( json_writer_ctx_t *context )
{
  apr_array_pop( context->prop_stack );
  apr_array_pop( context->state_stack );
  return TRUE;
}

int json_writer_ctx_can_write_value( json_writer_ctx_t *context )
{
  json_state state = json_writer_ctx_get_state( context );
  return ( state == JSON_PROPERTY || state == JSON_IN_ARRAY );
}

json_writer_t *json_writer_create( apr_pool_t *mp )
{
  json_writer_t *writer;
  writer = apr_palloc( mp, sizeof( json_writer_t ) );
  writer->mp = mp;
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

  if ( writer->json_stack->nelts > 0 ) {
    obj = APR_ARRAY_TAIL( writer->json_stack, json_t * );
  }

  if ( !obj ) {
    writer->json = json;
    return;
  }

  switch ( obj->type ) {
  case JSON_OBJECT:
    JSON_NAME( json ) = json_writer_ctx_get_prop( writer->context );
    tmp_json = apr_hash_get( obj->value.object, JSON_NAME( json ),
			     APR_HASH_KEY_STRING );
    if ( tmp_json && tmp_json->type != JSON_ARRAY ) {
      /* Key already exists, make an array and put both objects in it. */
      new_array = json_create_array( writer->mp );
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

  json = json_create_object( writer->mp );
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

  json = json_create_array( writer->mp );
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

  json_add( writer, json_create_string( writer->mp, value ) );
}

void json_writer_write_integer( void *writer_ptr, int value )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_can_write_value( writer->context ) ) {
    json_writer_error( "could not write int \"%d\"", value );
    return;
  }

  json_add( writer, json_create_integer( writer->mp, value ) );
}

void json_writer_write_number( void *writer_ptr, double value )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_can_write_value( writer->context ) ) {
    json_writer_error( "could not write number \"%lf\"", value );
    return;
  }

  json_add( writer, json_create_number( writer->mp, value ) );
}

void json_writer_write_boolean( void *writer_ptr, int value )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_can_write_value( writer->context ) ) {
    json_writer_error( "could not write bool" );
    return;
  }

  json_add( writer, json_create_boolean( writer->mp, value ) );
}

void json_writer_write_null( void *writer_ptr )
{
  json_writer_t *writer = (json_writer_t *) writer_ptr;
  if ( !json_writer_ctx_can_write_value( writer->context ) ) {
    json_writer_error( "could not write null" );
    return;
  }

  json_add( writer, json_create_null( writer->mp ) );
}
