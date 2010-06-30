#include <apr_general.h>

#include "apr_macros.h"
#include "json_path.h"
#include "json_parser.h"
#include "json.h"
#include "json_node.h"


int main( int argc, char **argv )
{
  json_writer_t writer;
  json_path_obj_t *path;
  json_t *json;

  apr_app_initialize( NULL, NULL, NULL );

  json_writer_init( &writer );
  json_file_parse( argv[1], &writer );

  path = json_path_obj_create( NULL );
  json_path_eval( argv[2], writer.json, path );

  int i;
  for ( i = 0; i < path->nodes->nelts; i++ ) {
    json = APR_ARRAY_IDX( path->nodes, i, json_t * );
    json_object_print( json, 1 );
  }

  json_path_eval( argv[3], writer.json, path );

  for ( i = 0; i < path->nodes->nelts; i++ ) {
    json = APR_ARRAY_IDX( path->nodes, i, json_t * );
    json_object_print( json, 1 );
  }

  apr_terminate();

  return 0;
}
