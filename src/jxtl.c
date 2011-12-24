/*
 * $Id$
 *
 * Description
 *  The jxtl command line processor.
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

#include <apr_general.h>
#include <apr_getopt.h>
#include <apr_lib.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"

#include "json.h"
#include "jxtl_path.h"
#include "json_writer.h"
#include "parser.h"

#include "jxtl.h"
#include "jxtl_parse.h"
#include "jxtl_lex.h"
#include "jxtl_template.h"
#include "xml2json.h"

typedef struct format_data_t {
  apr_pool_t *mp;
  apr_array_header_t *string_array;
} format_data_t;

static char *format_upper( json_t *json, char *format, void *format_data_ptr )
{
  format_data_t *format_data = (format_data_t *) format_data_ptr;
  char *value = json_get_string_value( format_data->mp, json );
  char *c;

  for ( c = value; c && *c; c++ ) {
    *c = apr_toupper( *c );
  }

  return value;
}

static char *format_lower( json_t *json, char *format, void *format_data_ptr )
{
  format_data_t *format_data = (format_data_t *) format_data_ptr;
  char *value;
  char *c;

  value = json_get_string_value( format_data->mp, json );

  for ( c = value; c && *c; c++ ) {
    *c = apr_tolower( *c );
  }

  return value;
}

static char *format_trn_field( json_t *json, char *format,
                               void *format_data_ptr )
{
  format_data_t *format_data = (format_data_t *) format_data_ptr;
  char *value;
  char *ret_value;
  char *c;
  int len = 0;

  APR_ARRAY_CLEAR( format_data->string_array );
  value = json_get_string_value( format_data->mp, json );

  if ( value ) {
    len = strlen( value );
  }

  for ( c = value; c && *c; c++ ) {
    if ( *c == '\'' ) {
      APR_ARRAY_PUSH( format_data->string_array, char ) = '\'';
    }
    APR_ARRAY_PUSH( format_data->string_array, char ) = *c;
  }
  APR_ARRAY_PUSH( format_data->string_array, char ) = '\0';
  
  if ( len > 80 ) {
    ret_value = apr_pstrcat( format_data->mp,
                             apr_psprintf( format_data->mp, "%d", len ),
                             "'", format_data->string_array->elts,
                             "'", NULL );
  }
  else {
    ret_value = apr_pstrcat( format_data->mp, "'",
                             format_data->string_array->elts,
                             "'", NULL );
  }

  return ret_value;
}

static char *format_json( json_t *json, char *format, void *format_data_ptr )
{
  format_data_t *format_data = (format_data_t *) format_data_ptr;
  char *value;
  char *ret_value;
  unsigned char *c;

  APR_ARRAY_CLEAR( format_data->string_array );
  value = json_get_string_value( format_data->mp, json );
  ret_value = value;

  if ( value ) {
    for ( c = (unsigned char *) value; c && *c; c++ ) {
      if ( *c > 0x1F ) {
        if ( *c == '\\' || *c == '/' || *c == '"' ) {
          APR_ARRAY_PUSH( format_data->string_array, char ) = '\\';
        }
        APR_ARRAY_PUSH( format_data->string_array, char ) = *c;
      }
      else {
        APR_ARRAY_PUSH( format_data->string_array, char ) = '\\';
        switch ( *c ) {
        case '\b':
          APR_ARRAY_PUSH( format_data->string_array, char ) = 'b';
          break;
        case '\f':
          APR_ARRAY_PUSH( format_data->string_array, char ) = 'f';
          break;
        case '\n':
          APR_ARRAY_PUSH( format_data->string_array, char ) = 'n';
          break;
        case '\r':
          APR_ARRAY_PUSH( format_data->string_array, char ) = 'r';
          break;
        case '\t':
          APR_ARRAY_PUSH( format_data->string_array, char ) = 't';
          break;
        }
      }
    }
    APR_ARRAY_PUSH( format_data->string_array, char ) = '\0';
    ret_value = apr_pstrdup( format_data->mp,
                             (char *) format_data->string_array->elts );
  }

  return ret_value;
}

void jxtl_usage( const char *prog_name,
                 const apr_getopt_option_t *options )
{
  int i;

  printf( "Usage: %s [options]\n", prog_name );
  printf( "  Options:\n" );
  for ( i = 0; options[i].name; i++ ) {
    printf( "    -%c, --%s %s\n", options[i].optch, options[i].name,
            options[i].description );
  }
}

/**
 * Read in the command line arguments to set the template file and the
 * data_file.
 */
void jxtl_init( int argc, char const * const *argv, apr_pool_t *mp,
                const char **template_file, const char **json_file,
                const char **xml_file, int *skip_root,
                const char **output_file )
{
  apr_getopt_t *options;
  apr_status_t ret;
  int ch;
  const char *arg;
  const apr_getopt_option_t jxtl_options[] = {
    { "template", 't', 1, "template file" },
    { "json", 'j', 1, "JSON data dictionary for template" },
    { "xml", 'x', 1, "XML data dictionary for template" },
    { "skiproot", 's', 0,
      "Skip the root element if using an XML data dictionary" },
    { "output", 'o', 1, "file to save output to" },
    { 0, 0, 0, 0 }
  };

  *template_file = NULL;
  *json_file = NULL;
  *xml_file = NULL;
  *skip_root = FALSE;
  *output_file = NULL;

  apr_getopt_init( &options, mp, argc, argv );

  while ( ( ret = apr_getopt_long( options, jxtl_options, &ch,
                                   &arg ) ) == APR_SUCCESS ) {
    switch ( ch ) {
    case 'j':
      *json_file = arg;
      break;

    case 'x':
      *xml_file = arg;
      break;

    case 's':
      *skip_root = TRUE;
      break;
      
    case 'o':
      *output_file = arg;
      break;

    case 't':
      *template_file = arg;
      break;
    }
  }

  if ( ( ret == APR_BADCH ) || ( *template_file == NULL ) ||
       ( ( *json_file == NULL ) && ( *xml_file == NULL ) ) ) {
    jxtl_usage( argv[0], jxtl_options );
    exit( EXIT_FAILURE );
  }
}

/**
 * Load data from either json_file or xml_file.  One of those has to be
 * non-null.
 */
static int load_data( apr_pool_t *mp, const char *json_file,
                      const char *xml_file, int skip_root, json_t **obj )
{
  int ret;
  parser_t *json_parser;

  if ( xml_file ) {
    ret = xml_file_to_json( mp, xml_file, skip_root, obj );
  }
  else {
    json_parser = json_parser_create( mp );
    ret = json_parser_parse_file( json_parser, json_file, obj );
  }

  return ret;
}

static int open_output_file( apr_pool_t *mp, const char *file,
                             apr_file_t **out )
{
  int status;

  if ( !file || apr_strnatcasecmp( file, "-" ) == 0 ) {
    status = apr_file_open_stdout( out, mp );
  }
  else {
    status = apr_file_open( out, file, APR_WRITE | APR_CREATE | APR_BUFFERED |
                            APR_TRUNCATE, APR_OS_DEFAULT, mp );
  }

  return ( status == APR_SUCCESS );
}

int main( int argc, char const * const *argv )
{
  apr_pool_t *mp;
  const char *template_file = NULL;
  const char *json_file = NULL;
  const char *xml_file = NULL;
  const char *out_file = NULL;
  int skip_root;
  json_t *json;
  parser_t *jxtl_parser;
  jxtl_template_t *template;
  format_data_t *format_data;
  apr_file_t *out;
  int is_stdout;

  apr_app_initialize( NULL, NULL, NULL );
  apr_pool_create( &mp, NULL );

  jxtl_init( argc, argv, mp, &template_file, &json_file, &xml_file,
             &skip_root, &out_file );

  jxtl_parser = jxtl_parser_create( mp );

  if ( load_data( mp, json_file, xml_file, skip_root, &json ) &&
       open_output_file( mp, out_file, &out ) &&
       jxtl_parser_parse_file_to_template( mp, jxtl_parser, template_file,
					   &template ) ) {
    format_data = apr_palloc( mp, sizeof(format_data_t) );
    format_data->mp = mp;
    format_data->string_array = apr_array_make( mp, 8192, sizeof(char) );
    jxtl_template_register_format( template, "upper", format_upper );
    jxtl_template_register_format( template, "lower", format_lower );
    jxtl_template_register_format( template, "trn_field", format_trn_field);
    jxtl_template_register_format( template, "json", format_json );
    jxtl_template_set_format_data( template, format_data );
    jxtl_template_expand_to_file( template, json, out );
  }

  apr_file_close( out );
  apr_pool_destroy( mp );
  apr_terminate();

  return 0;
}
