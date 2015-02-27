#include <stdio.h>
#include <stdlib.h>
#include <apr_getopt.h>
#include <apr_pools.h>

#include "json.h"
#include "xml2json.h"
#include "json_writer.h"
#include "misc.h"

void xml2json_init( int argc, char const * const *argv, apr_pool_t *mp,
                    const char **xml_file, const char **output_file,
                    int *preserve_root, int *indent )
{
  apr_getopt_t *options;
  apr_status_t ret;
  int ch;
  const char *arg;
  const apr_getopt_option_t xml2json_options[] = {
    { "indent", 'i', 0, "indent the JSON output" },
    { "output", 'o', 1, "file to save output to" },
    { "preserve-root", 'p', 0, "preserve the root element of XML file" },
    { "xml_file", 'x', 1, "XML file" },
    { 0, 0, 0, 0 }
  };

  *xml_file = "-";
  *preserve_root = FALSE;
  *output_file = "-";
  *indent = FALSE;

  apr_getopt_init( &options, mp, argc, argv );

  while ( ( ret = apr_getopt_long( options, xml2json_options, &ch,
                                   &arg ) ) == APR_SUCCESS ) {
    switch ( ch ) {
    case 'i':
      *indent = TRUE;
      break;

    case 'o':
      *output_file = arg;
      break;

    case 'p':
      *preserve_root = TRUE;
      break;
      
    case 'x':
      *xml_file = arg;
      break;
    }
  }

  if ( ret == APR_BADCH ) {
    print_usage( argv[0], xml2json_options );
    exit( EXIT_FAILURE );
  }
}

int main( int argc, char const * const *argv )
{
  json_t *json;
  int ret;
  apr_status_t status;
  apr_pool_t *mp;
  apr_file_t *xml_fp;
  apr_file_t *out_fp;
  const char *xml_file;
  const char *out_file;
  int preserve_root;
  int indent;

  apr_app_initialize( NULL, NULL, NULL );
  apr_pool_create( &mp, NULL );

  xml2json_init( argc, argv, mp, &xml_file, &out_file, &preserve_root,
                 &indent );

  if ( open_apr_input_file( mp, xml_file, &xml_fp ) &&
       open_apr_output_file( mp, out_file, &out_fp ) &&
       xml_to_json( mp, xml_fp, !preserve_root, &json ) ) {
    json_dump( out_fp, json, indent );
    ret = 0;
  }
  else {
    fprintf( stderr, "failed to convert\n" );
    ret = 1;
  }

  apr_pool_destroy( mp );
  apr_terminate();

  return ret;
}
