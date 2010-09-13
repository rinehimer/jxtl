#include <apr_general.h>
#include <apr_pools.h>

#include "jxtl.h"
#include "jxtl_path.h"
#include "json.h"

static void json_print_value( json_t *json, apr_file_t *out )
{
  if ( !json )
    return;

  switch ( json->type ) {
  case JSON_STRING:
    apr_file_printf( out, "%s", json->value.string );
    break;
    
  case JSON_INTEGER:
    apr_file_printf( out, "%d", json->value.integer );
    break;
    
  case JSON_NUMBER:
    apr_file_printf( out, "%g", json->value.number );
    break;

  case JSON_BOOLEAN:
    apr_file_printf( out, "%s",
                     JSON_IS_TRUE_BOOLEAN( json ) ? "true" : "false" );
    break;

  default:
    fprintf( stderr, "error: cannot get value of object or array\n" );
    break;
  }
}

static void text_print( char *text, jxtl_content_t *prev_content,
                        jxtl_content_t *next_content,
                        section_print_type print_type,
                        apr_file_t *out )
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
  apr_file_printf( out, "%.*s", len, text_ptr );
}

static void jxtl_print_content( apr_pool_t *mp,
                                apr_array_header_t *content_array,
                                json_t *json, section_print_type print_type,
                                apr_file_t *out );

/**
 * Print a saved section
 */
static void jxtl_print_section( apr_pool_t *mp,
                                jxtl_section_t *section,
                                json_t *json,
                                section_print_type print_type,
                                apr_file_t *out )
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
    jxtl_print_content( mp, section->content, json_value, PRINT_SECTION, out );
    num_printed++;
    /* Only print the separator if it's not the last one */
    if ( num_printed < num_items )
      jxtl_print_content( mp, section->separator, json_value, PRINT_SEPARATOR,
                          out );
  }
}

static void jxtl_print_content( apr_pool_t *mp,
                                apr_array_header_t *content_array,
                                json_t *json, section_print_type print_type,
                                apr_file_t *out )
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
      text_print( content->value, prev_content, next_content, print_type,
                  out );
      break;

    case JXTL_SECTION:
      tmp_section = (jxtl_section_t *) content->value;
      jxtl_print_section( mp, tmp_section, json, PRINT_SECTION, out );
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
          jxtl_print_content( mp, jxtl_if->content, json, PRINT_SECTION, out );
          break;
        }
      }
      break;

    case JXTL_VALUE:
      if ( jxtl_path_compiled_eval( mp, content->value, json, &path_obj ) ) {
        json_value = APR_ARRAY_IDX( path_obj->nodes, 0, json_t * );
        json_print_value( json_value, out );
      }
      break;
    }
    prev_content = content;
  }
}

int jxtl_print_content_to_file( apr_pool_t *mp,
                                 apr_array_header_t *content_array,
                                 json_t *json, const char *file )
{
  apr_file_t *out;
  apr_status_t status;

  if ( file ) {
    status = apr_file_open( &out, file,
                            APR_WRITE | APR_CREATE | APR_BUFFERED |
                            APR_TRUNCATE, APR_OS_DEFAULT, mp );
  }
  else {
    status = apr_file_open_stdout( &out, mp );
  }

  if ( status == APR_SUCCESS ) {
    jxtl_print_content( mp, content_array, json, PRINT_NORMAL, out );
  }
  return ( status == APR_SUCCESS ) ? TRUE : FALSE;
}
