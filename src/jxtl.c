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
#include "xml2json.h"

static char *format_func( char *value, char *format_name, void *user_data )
{
  apr_array_header_t *format_buf;
  char *c;

  format_buf = (apr_array_header_t *) user_data;
  APR_ARRAY_CLEAR( format_buf );

  if ( apr_strnatcasecmp( format_name, "upper" ) == 0 ) {
    for ( c = value; *c; c++ ) {
      APR_ARRAY_PUSH( format_buf, char ) = apr_toupper( *c );
    }
  }
  else if ( apr_strnatcasecmp( format_name, "lower" ) == 0 ) {
    for ( c = value; *c; c++ ) {
      APR_ARRAY_PUSH( format_buf, char ) = apr_tolower( *c );
    }
  }
  APR_ARRAY_PUSH( format_buf, char ) = '\0';

  return (char *) format_buf->elts;
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
  *skip_root = 0;
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
      *skip_root = 1;
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
int jxtl_load_data( apr_pool_t *mp, const char *json_file,
                    const char *xml_file, int skip_root, json_t **obj )
{
  int ret = 1;
  json_writer_t *writer = json_writer_create( mp );
  parser_t *json_parser;

  if ( xml_file ) {
    ret = xml_file_to_json( xml_file, writer, skip_root );
    *obj = writer->json;
  }
  else if ( json_file ) {
    json_parser = json_parser_create( mp );
    ret = json_parser_parse_file( json_parser, json_file, obj );
  }

  return ret;
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
  apr_array_header_t *format_buf;

  apr_app_initialize( NULL, NULL, NULL );
  apr_pool_create( &mp, NULL );

  jxtl_init( argc, argv, mp, &template_file, &json_file, &xml_file,
             &skip_root, &out_file );

  jxtl_parser = jxtl_parser_create( mp );

  if ( ( jxtl_load_data( mp, json_file, xml_file, skip_root,
                         &json ) == APR_SUCCESS ) &&
       ( jxtl_parser_parse_file( jxtl_parser, template_file,
                                 &template ) == APR_SUCCESS ) ) {
    format_buf = apr_array_make( mp, 8192, sizeof(char) );
    jxtl_template_set_format_func( template, format_func );
    jxtl_template_set_user_data( template, format_buf );
    jxtl_expand_to_file( template, json, out_file );
  }

  apr_pool_destroy( mp );
  apr_terminate();

  return 0;
}
