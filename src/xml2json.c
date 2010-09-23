#include <stdio.h>
#include <stdlib.h>
#include <apr_pools.h>

#include "json.h"
#include "xml2json.h"
#include "json_writer.h"

void xml2json_usage( const char *prog_name )
{
  fprintf( stderr, "Usage: %s <file>\n", prog_name );
  fprintf( stderr, "<file> can be \"-\" to signify stdin\n" );
  exit( 1 );
}

int main( int argc, char **argv )
{
  json_writer_t *writer;
  int ret;
  apr_pool_t *mp;

  apr_app_initialize( NULL, NULL, NULL );
  apr_pool_create( &mp, NULL );
  writer = json_writer_create( mp );

  if ( argc < 2 ) {
    xml2json_usage( argv[0] );
  }
  
  if ( xml_file_to_json( argv[1], writer, 1 ) == 0 ) {
    json_dump( writer->json, 1 );
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
