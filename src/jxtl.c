#include <apr_general.h>
#include <apr_getopt.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"

#include "json.h"
#include "jxtl_path.h"
#include "json_writer.h"
#include "json_parser.h"
#include "parser.h"

#include "jxtl.h"
#include "jxtl_parse.h"
#include "jxtl_lex.h"
#include "xml2json.h"

/** Constants used for calling the print function */
typedef enum section_print_type {
  PRINT_NORMAL,
  PRINT_SECTION,
  PRINT_SEPARATOR
} section_print_type;

static void json_value_print( json_t *json )
{
  if ( !json )
    return;

  switch ( json->type ) {
  case JSON_STRING:
    printf( "%s", json->value.string );
    break;
    
  case JSON_INTEGER:
    printf( "%d", json->value.integer );
    break;
    
  case JSON_NUMBER:
    printf( "%g", json->value.number );
    break;

  case JSON_BOOLEAN:
    printf( "%s", JSON_IS_TRUE_BOOLEAN( json ) ? "true" : "false" );
    break;

  default:
    fprintf( stderr, "error: cannot get value of object or array\n" );
    break;
  }
}

static void text_print( char *text, jxtl_content_t *prev_content,
                        jxtl_content_t *next_content,
                        section_print_type print_type )
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
  printf( "%.*s", len, text_ptr );
}

static void jxtl_section_print( apr_pool_t *mp,
                                jxtl_section_t *section,
                                json_t *json,
                                section_print_type print_type );

static void jxtl_content_print( apr_pool_t *mp,
                                apr_array_header_t *content_array,
                                json_t *json,
                                section_print_type print_type )
{
  int i, j;
  jxtl_content_t *content, *prev_content, *next_content;
  jxtl_section_t *tmp_section;
  json_t *json_value;
  jxtl_if_t *jxtl_if;
  apr_array_header_t *if_block;
  jxtl_path_obj_t *path_obj;

  if ( !json )
    return;

  prev_content = NULL;
  next_content = NULL;

  for ( i = 0; i < content_array->nelts; i++ ) {
    content = APR_ARRAY_IDX( content_array, i, jxtl_content_t * );
    next_content = ( i + 1 < content_array->nelts ) ?
                   APR_ARRAY_IDX( content_array, i + 1, jxtl_content_t * ) :
                   NULL;
    switch ( content->type ) {
    case JXTL_TEXT:
      text_print( content->value, prev_content, next_content, print_type );
      break;

    case JXTL_SECTION:
      tmp_section = (jxtl_section_t *) content->value;
      jxtl_section_print( mp, tmp_section, json, PRINT_SECTION );
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
             jxtl_path_compiled_eval( mp, jxtl_if->expr, json, &path_obj ) ) {
          jxtl_content_print( mp, jxtl_if->content, json, PRINT_SECTION );
          break;
        }
      }
      break;

    case JXTL_VALUE:
      if ( jxtl_path_compiled_eval( mp, content->value, json, &path_obj ) ) {
        json_value = APR_ARRAY_IDX( path_obj->nodes, 0, json_t * );
        json_value_print( json_value );
      }
      break;
    }
    prev_content = content;
  }
}

/**
 * Print a saved section
 */
static void jxtl_section_print( apr_pool_t *mp,
                                jxtl_section_t *section,
                                json_t *json,
                                section_print_type print_type )
{
  int i;
  int num_items;
  int num_printed;
  json_t *json_value;
  jxtl_path_obj_t *path_obj;

  if ( !json )
    return;

  num_items = jxtl_path_compiled_eval( mp, section->expr, json, &path_obj );
  num_printed = 0;
  for ( i = 0; i < path_obj->nodes->nelts; i++ ) {
    json_value = APR_ARRAY_IDX( path_obj->nodes, i, json_t * );
    jxtl_content_print( mp, section->content, json_value, PRINT_SECTION );
    num_printed++;
    /* Only print the separator if it's not the last one */
    if ( num_printed < num_items )
      jxtl_content_print( mp, section->separator, json_value, PRINT_SEPARATOR );
  }
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
void jxtl_init( int argc, char const * const *argv , apr_pool_t *mp,
                const char **template_file, const char **json_file,
                const char **xml_file, int *skip_root )
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
    { 0, 0, 0, 0 }
  };

  *template_file = NULL;
  *json_file = NULL;
  *xml_file = NULL;
  *skip_root = 0;

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
  int skip_root;
  json_t *json;
  parser_t *jxtl_parser;
  apr_array_header_t *content_array;

  apr_app_initialize( NULL, NULL, NULL );
  apr_pool_create( &mp, NULL );

  jxtl_init( argc, argv, mp, &template_file, &json_file, &xml_file,
             &skip_root );

  jxtl_parser = jxtl_parser_create( mp );

  if ( ( jxtl_load_data( mp, json_file, xml_file, skip_root,
                         &json ) == APR_SUCCESS ) &&
       ( jxtl_parser_parse_file( jxtl_parser, template_file,
                                 &content_array ) == APR_SUCCESS ) ) {
    jxtl_content_print( mp, content_array, json, PRINT_NORMAL );
  }

  apr_pool_destroy( mp );
  apr_terminate();

  return 0;
}
