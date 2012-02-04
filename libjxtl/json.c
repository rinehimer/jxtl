/*
 * json.c
 *
 * Description
 *   Functions for creating and dumping JSON.
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
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"

#include "json.h"

#include "json_parse.h"
#include "json_lex.h"
#include "json_writer.h"

#define JSON_CREATE( mp, json )                                    \
  json = apr_palloc( mp, sizeof(json_t) );                         \
  json->name = NULL;                                               \
  json->parent = NULL

json_t *json_create_string( apr_pool_t *mp, unsigned char *string )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.string = (unsigned char *) apr_pstrdup( mp, (char *) string );
  json->type = JSON_STRING;
  return json;
}

json_t *json_create_integer( apr_pool_t *mp, int integer )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.integer = integer;
  json->type = JSON_INTEGER;
  return json;
}

json_t *json_create_number( apr_pool_t *mp, double number )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.number = number;
  json->type = JSON_NUMBER;
  return json;
}

json_t *json_create_object( apr_pool_t *mp )
{
  json_t *json;

  JSON_CREATE( mp, json );
  json->value.object = apr_hash_make( mp );
  json->type = JSON_OBJECT;
  return json;
}

json_t *json_create_array( apr_pool_t *mp )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.array = apr_array_make( mp, 8, sizeof(json_t *) );
  json->type = JSON_ARRAY;
  return json;
}

json_t *json_create_boolean( apr_pool_t *mp, int boolean )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.boolean = boolean;
  json->type = JSON_BOOLEAN;
  return json;
}

json_t *json_create_null( apr_pool_t *mp )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->type = JSON_NULL;
  return json;
}

static void initialize_callbacks( apr_pool_t *json_mp, apr_pool_t *tmp_mp,
                                  json_callback_t *callback_data, 
                                  json_writer_t *writer )
{
  callback_data->object_start_handler = json_writer_start_object;
  callback_data->object_end_handler = json_writer_end_object;
  callback_data->array_start_handler = json_writer_start_array;
  callback_data->array_end_handler = json_writer_end_array;
  callback_data->property_start_handler = json_writer_start_property;
  callback_data->property_end_handler = json_writer_end_property;
  callback_data->string_handler = json_writer_write_string;
  callback_data->integer_handler = json_writer_write_integer;
  callback_data->number_handler = json_writer_write_number;
  callback_data->boolean_handler = json_writer_write_boolean;
  callback_data->null_handler = json_writer_write_null;
  callback_data->user_data = writer;
}

static int parse_file_or_buffer( apr_pool_t *mp, parser_t *parser,
                                 const char *file_or_buf,
                                 int ( *parse_func )( parser_t *,
                                                      const char *,
                                                      json_callback_t * ),
                                 json_t **obj )
{
  apr_pool_t *tmp_mp;
  json_writer_t *writer;
  json_callback_t callback_data;
  int result = FALSE;

  apr_pool_create( &tmp_mp, NULL );
  writer = json_writer_create( tmp_mp, mp );

  initialize_callbacks( mp, tmp_mp, &callback_data, writer );

  *obj = NULL;
  if ( parse_func( parser, file_or_buf, &callback_data ) ) {
    *obj = writer->json;
    result = TRUE;
  }

  apr_pool_destroy( tmp_mp );
  return result;
}

int json_parser_parse_file_to_obj( apr_pool_t *mp, parser_t *parser,
                                   const char *file, json_t **obj )
{
  return parse_file_or_buffer( mp, parser, file, json_parser_parse_file,
                               obj );
}

int json_parser_parse_buffer_to_obj( apr_pool_t *mp, parser_t *parser,
                                     const char *buffer, json_t **obj )
{
  return parse_file_or_buffer( mp, parser, buffer, json_parser_parse_buffer,
                               obj );
}

static void print_spaces( int num )
{
  while ( num-- > 0 ) {
    printf( " " );
  }
}

static void print_string( unsigned char *str )
{
  unsigned char *c = str;

  printf( "\"" );
  while ( *c ) {
    if ( *c < 32 ) {
      printf( "\\" );
      switch ( *c ) {
      case '\b':
        printf( "b" );
        break;
      case '\t':
        printf( "t" );
        break;
      case '\n':
        printf( "n" );
        break;
      case '\f':
        printf( "f" );
        break;
      case '\r':
        printf( "r" );
        break;
      default:
        printf( "u%.4x", *c );
        break;
      }
    }
    else if ( *c == '\\' ) {
      printf( "\\\\" );
    }
    else if ( *c == '"' ) {
      printf( "\\\"" );
    }
    else
      printf( "%c", *c );
    c++;
  }
  printf( "\"" );
}

static void dump_internal( json_t *json, int first, int depth, int indent )
{
  apr_array_header_t *arr = NULL;
  int i = 0;
  json_t *tmp_json = NULL;
  apr_hash_index_t *idx;

  if ( !first )
    printf( "," );

  if ( ( depth > 0 ) && indent ) {
    printf( "\n" );
    print_spaces( depth * indent );
  }

  if ( JSON_NAME( json ) ) {
    print_string( JSON_NAME( json ) );
    printf( ":" );
     if ( indent )
      print_spaces( 1 );
  }

  switch ( json->type ) {
  case JSON_STRING:
    print_string( json->value.string );
    break;

  case JSON_INTEGER:
    printf( "%d", json->value.integer );
    break;

  case JSON_NUMBER:
    printf( "%g", json->value.number );
    break;

  case JSON_OBJECT:
    printf( "{" );
    for ( i = 0, idx = apr_hash_first( NULL, json->value.object ); idx;
          i++, idx = apr_hash_next( idx ) ) {
      apr_hash_this( idx, NULL, NULL, (void **) &tmp_json );
      dump_internal( tmp_json, i == 0, depth + 1, indent );
    }

    if ( indent && i > 0 ) {
      printf( "\n" );
      print_spaces( depth * indent );
    }
    printf( "}" );
    break;

  case JSON_ARRAY:
    arr = json->value.array;
    printf( "[" );
    for ( i = 0; arr && i < arr->nelts; i++ ) {
      tmp_json = APR_ARRAY_IDX( arr, i, json_t * );
      dump_internal( tmp_json, i == 0, depth + 1, indent );
    }

    if ( indent && i > 0 ) {
      printf( "\n" );
      print_spaces( depth * indent );
    }

    printf( "]" );
    break;

  case JSON_BOOLEAN:
    ( json->value.boolean ) ? printf( "true" ) :
                              printf( "false" );
    break;

  case JSON_NULL:
    printf( "null" );
    break;

  default:
    fprintf( stderr, "error:  unrecognized object type\n" );
    break;
  }

  if ( ( depth == 0 ) && indent ) {
    printf( "\n" );
  }
}

/*
 * Externally visible function that invokes the internal print function.
 */
void json_dump( json_t *json, int indent )
{
  dump_internal( json, 1, 0, indent );
}

static void print_xml_string( unsigned char *str )
{
  unsigned char *c = str;

  while ( *c ) {
    if ( ( *c < 0x20 ) && ( *c != 0x9 ) && ( *c != 0xa ) && ( *c != 0xd ) ) {
      /*
       * XML can't handle these characters, so just store as the Unicode escape
       * sequence.
       */
      printf( "\\u%.4x", *c );
    }
    else
      printf( "%c", *c );
    c++;
  }
}

static void json_to_xml_internal( json_t *json, int indent )
{
  json_t *tmp_json;
  apr_array_header_t *arr;
  unsigned char *json_name = JSON_NAME( json );
  apr_hash_index_t *idx;
  int i;

  if ( json_name && json->type != JSON_ARRAY ) {
    print_spaces( indent );
    printf( "<%s>", json_name );
  }

  switch ( json->type ) {
  case JSON_STRING:
    print_xml_string( json->value.string );
    break;

  case JSON_INTEGER:
    printf( "%d", json->value.integer );
    break;

  case JSON_NUMBER:
    printf( "%lf", json->value.number );
    break;

  case JSON_OBJECT:
    for ( idx = apr_hash_first( NULL, json->value.object ); idx;
          idx = apr_hash_next( idx ) ) {
      apr_hash_this( idx, NULL, NULL, (void **) &tmp_json );
      json_to_xml_internal( tmp_json, indent + 1 );
    }
    break;

  case JSON_ARRAY:
    arr = json->value.array;

    for ( i = 0; i < arr->nelts; i++ ) {
      tmp_json = APR_ARRAY_IDX( arr, i, json_t * );
      print_spaces( indent );
      printf( "<%s>", json_name );
      json_to_xml_internal( tmp_json, indent + 1 );
      printf( "</%s>\n", json_name );
    }
    break;

  case JSON_BOOLEAN:
    ( json->value.boolean ) ?
      printf( "true" ) :
      printf( "false" );
    break;

  case JSON_NULL:
    printf( "null" );
    break;

  default:
    fprintf( stderr, "error:  unrecognized object type\n" );
    break;
  }

  if ( json_name && json->type != JSON_ARRAY ) {
    printf( "</%s>\n", json_name );
  }
}

/*
 * Figure out if I really want to keep this around, it was really more for
 * debugging early on.  There could potentially be some use, but it's possible
 * the XML produced is invalid since it doesn't use an API to generate it.
 */
void json_to_xml( json_t *json, int indent )
{
  printf( "<?xml version=\"1.0\"?>\n" );
  printf( "<json>\n" );
  json_to_xml_internal( json, 1 );
  printf( "</json>\n" );
}

char *json_get_string_value( apr_pool_t *mp, json_t *json )
{
  char *value = NULL;

  switch ( json->type ) {
  case JSON_STRING:
    value = apr_psprintf( mp, "%s", json->value.string );
    break;

  case JSON_INTEGER:
    value = apr_psprintf( mp, "%d", json->value.integer );
    break;

  case JSON_NUMBER:
    value = apr_psprintf( mp, "%g", json->value.number );
    break;

  case JSON_BOOLEAN:
    value = apr_psprintf( mp, "%s",
                          JSON_IS_TRUE_BOOLEAN( json ) ? "true" : "false" );
    break;

  default:
    break;
  }

  return value;
}
