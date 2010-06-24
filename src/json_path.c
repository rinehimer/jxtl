#include <apr_general.h>

#include "json_path.h"
#include "json_parser.h"


int main( int argc, char **argv )
{
  json_writer_t writer;

  apr_app_initialize( NULL, NULL, NULL );

  json_writer_init( &writer );
  json_file_parse( argv[1], &writer );

  json_path_evaluate( argv[2], writer.json );

  apr_terminate();

  return 0;
}
