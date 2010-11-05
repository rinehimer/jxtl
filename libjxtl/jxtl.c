/*
 * $Id$
 *
 * Description
 *   Contains the functions for expanding a template.
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

#include <apr_buckets.h>
#include <apr_general.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "jxtl.h"
#include "jxtl_path.h"
#include "json.h"

/** Constants used for calling the expand functions */
typedef enum section_print_type {
  PRINT_NORMAL,
  PRINT_SECTION,
  PRINT_SEPARATOR
} section_print_type;

static void print_json_value( json_t *json,
                              char *format,
                              apr_pool_t *mp,
                              jxtl_template_t *template,
                              apr_bucket_brigade *out )
{
  char *value = NULL;

  if ( !json )
    return;
  
  if ( format && template->format ) {
    value = template->format( json, format, template->format_data );
  }
  else {
    value = json_get_string_value( mp, json );
  }

  if ( value ) {
    apr_brigade_printf( out, NULL, NULL, "%s", value );
  }
}

static void print_text( char *text, jxtl_content_t *prev_content,
                        jxtl_content_t *next_content,
                        section_print_type print_type,
                        apr_bucket_brigade *out )
{
  char *text_ptr = text;
  int len = strlen( text_ptr );

  if ( ( print_type == PRINT_SECTION ) && ( !prev_content ) &&
       ( text_ptr[0] == '\n' ) ) {
    text_ptr++;
    len--;
  }
  if ( ( print_type == PRINT_SECTION ) && ( !next_content ) &&
       ( text_ptr[len - 1] == '\n' ) ) {
    len--;
  }
  apr_brigade_printf( out, NULL, NULL, "%.*s", len, text_ptr );
}

static void expand_content( apr_pool_t *mp,
                            jxtl_template_t *template,
                            apr_array_header_t *content_array,
                            json_t *json,
                            char *prev_format,
                            section_print_type print_type,
                            apr_bucket_brigade *out );

/**
 * Print a saved section
 */
static void expand_section( apr_pool_t *mp,
                            jxtl_template_t *template,
                            jxtl_section_t *section,
                            apr_array_header_t *separator,
                            json_t *json,
                            char *format,
                            section_print_type print_type,
                            apr_bucket_brigade *out )
{
  int i;
  int num_items;
  json_t *json_value;
  jxtl_path_obj_t *path_obj;

  if ( !json )
    return;

  num_items = jxtl_path_compiled_eval( mp, section->expr, json, &path_obj );
  for ( i = 0; i < path_obj->nodes->nelts; i++ ) {
    json_value = APR_ARRAY_IDX( path_obj->nodes, i, json_t * );
    expand_content( mp, template, section->content, json_value, format,
                    PRINT_SECTION, out );
    /* Only print the separator if it's not the last one */
    if ( separator && ( i + 1 < num_items ) ) {
      expand_content( mp, template, separator, json_value, format,
                      PRINT_SEPARATOR, out );
    }
  }
}

/**
 * Determine if the result of an if statement is true.  The criteria is:
 * 1) More than 1 node returned is automatically true.
 * 2) If there was exactly one node and it's not a boolean then it's true.
 * 3) If there was exactly one node and it is a boolean and it's value is true.
 * 4) Anything else is false.
 */
static int is_true_if( jxtl_path_obj_t *path_obj, int negate )
{
  json_t *json;
  int result = FALSE;

  if ( path_obj->nodes->nelts > 1 ) {
    result = TRUE;
  }
  else if ( path_obj->nodes->nelts == 1 ) {
    json = APR_ARRAY_HEAD( path_obj->nodes, json_t * );
    result = ( !JSON_IS_BOOLEAN( json ) || JSON_IS_TRUE_BOOLEAN( json ) );
  }
  
  return negate ? !result : result;
}

static void expand_content( apr_pool_t *mp,
                            jxtl_template_t *template,
                            apr_array_header_t *content_array,
                            json_t *json,
                            char *prev_format,
                            section_print_type print_type,
                            apr_bucket_brigade *out )
{
  int i, j;
  jxtl_content_t *content, *prev_content, *next_content;
  jxtl_section_t *tmp_section;
  json_t *json_value;
  jxtl_if_t *jxtl_if;
  apr_array_header_t *if_block;
  jxtl_path_obj_t *path_obj;
  char *format;

  prev_content = NULL;
  next_content = NULL;

  for ( i = 0; i < content_array->nelts; i++ ) {
    content = APR_ARRAY_IDX( content_array, i, jxtl_content_t * );
    next_content = ( i + 1 < content_array->nelts ) ?
                   APR_ARRAY_IDX( content_array, i + 1, jxtl_content_t * ) :
                   NULL;
    switch ( content->type ) {
    case JXTL_TEXT:
      print_text( content->value, prev_content, next_content, print_type,
                  out );
      break;

    case JXTL_SECTION:
      tmp_section = (jxtl_section_t *) content->value;
      format = ( content->format ) ? content->format : prev_format;
      expand_section( mp, template, tmp_section, content->separator, json,
                      format, PRINT_SECTION, out );
      break;

    case JXTL_IF:
      /*
       * Loop through all of the ifs until we find a true one and then break
       * the loop.
       */
      if_block = (apr_array_header_t *) content->value;
      for ( j = 0; j < if_block->nelts; j++ ) {
        jxtl_if = APR_ARRAY_IDX( if_block, j, jxtl_if_t * );
        if ( !jxtl_if->expr ||
             ( jxtl_path_compiled_eval( mp, jxtl_if->expr, json, &path_obj ) &&
               is_true_if( path_obj, jxtl_if->expr->negate ) ) ) {
          expand_content( mp, template, jxtl_if->content, json, prev_format,
                          PRINT_SECTION, out );
          break;
        }
      }
      break;

    case JXTL_VALUE:
      format = ( content->format ) ? content->format : prev_format;
      if ( jxtl_path_compiled_eval( mp, content->value, json, &path_obj ) ) {
        for ( j = 0; j < path_obj->nodes->nelts; j++ ) {
          json_value = APR_ARRAY_IDX( path_obj->nodes, j, json_t * );
          print_json_value( json_value, format, mp, template, out );
          if ( content->separator && ( j + 1 < path_obj->nodes->nelts ) ) {
            expand_content( mp, template, content->separator, json_value,
                            format, PRINT_SEPARATOR, out );
          }
        }
      }
      break;
    }
    prev_content = content;
  }
}

static int number_of_buckets( apr_bucket_brigade *b )
{
  apr_bucket *e;
  int n = 0;
  for ( e = APR_BRIGADE_FIRST( b );
        e != APR_BRIGADE_SENTINEL( b );
        e = APR_BUCKET_NEXT( e ) ) {
    n++;
  }

  return n;
}

void jxtl_template_set_format_func( jxtl_template_t *template,
                                    jxtl_format_func format_func )
{
  template->format = format_func;
}

void jxtl_template_set_format_data( jxtl_template_t *template,
                                    void *format_data )
{
  template->format_data = format_data;
}

int jxtl_expand_to_file( jxtl_template_t *template, json_t *json,
                         const char *file )
{
  apr_pool_t *mp;
  apr_file_t *out;
  apr_status_t status;
  apr_bucket_alloc_t *bucket_alloc;
  apr_bucket_brigade *bucket_brigade;
  struct iovec *vec;
  int nvec = 0;
  apr_size_t nbytes;
  int is_stdout;

  apr_pool_create( &mp, NULL );

  is_stdout = ( !file || apr_strnatcasecmp( file, "-" ) == 0 );

  if ( is_stdout ) {
    status = apr_file_open_stdout( &out, mp );
  }
  else {
    status = apr_file_open( &out, file,
                            APR_WRITE | APR_CREATE | APR_BUFFERED |
                            APR_TRUNCATE, APR_OS_DEFAULT, mp );
  }
  
  if ( status == APR_SUCCESS ) {
    bucket_alloc = apr_bucket_alloc_create( mp );
    bucket_brigade = apr_brigade_create( mp, bucket_alloc );

    expand_content( mp, template, template->content, json, NULL, PRINT_NORMAL,
                    bucket_brigade );

    nvec = number_of_buckets( bucket_brigade );
    vec = apr_palloc( mp, sizeof(struct iovec) * nvec );
    apr_brigade_to_iovec( bucket_brigade, vec, &nvec );
    status = apr_file_writev( out, vec, nvec, &nbytes );

    apr_brigade_destroy( bucket_brigade );
    apr_bucket_alloc_destroy( bucket_alloc );
    if ( !is_stdout ) {
      apr_file_close( out );
    }
  }

  apr_pool_destroy( mp );

  return ( status == APR_SUCCESS ) ? TRUE : FALSE;
}

char *jxtl_expand_to_buffer( apr_pool_t *mp, jxtl_template_t *template,
                             json_t *json )
{
  apr_bucket_alloc_t *bucket_alloc;
  apr_bucket_brigade *bucket_brigade;
  char *expanded_template;
  apr_off_t length;
  apr_size_t flatten_len;
  apr_pool_t *tmp_pool;

  apr_pool_create( &tmp_pool, NULL );

  bucket_alloc = apr_bucket_alloc_create( tmp_pool );
  bucket_brigade = apr_brigade_create( tmp_pool, bucket_alloc );
  
  expand_content( tmp_pool, template, template->content, json, NULL,
                  PRINT_NORMAL, bucket_brigade );

  apr_brigade_length( bucket_brigade, 1, &length );
  flatten_len = length;

  /* Allocate the string from the user's memory pool */
  expanded_template = apr_palloc( mp, length + 1 );
  apr_brigade_flatten( bucket_brigade, expanded_template, &flatten_len );
  expanded_template[length] = '\0';

  apr_brigade_destroy( bucket_brigade );
  apr_bucket_alloc_destroy( bucket_alloc );
  apr_pool_destroy( tmp_pool );

  return expanded_template;
}
