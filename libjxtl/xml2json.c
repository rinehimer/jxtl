#include <apr_general.h>
#include <apr_lib.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_xml.h>
#include <expat.h>

#include "json.h"
#include "json_writer.h"
#include "xml2json.h"
#include "str_buf.h"

static int str_is_whitespace( const char *str, int len )
{
  const char *c = str;
  
  while ( len-- > 0 ) {
    if ( !apr_isspace( *c ) ) {
      return FALSE;
    }
    c++;
  }

  return TRUE;
}

/**
 * When we have a string to write, check to see if it is a valid boolean value
 * in JSON.  It's possible we may do number checking here in the future.
 */
static void write_xml_strn( json_writer_t *writer, const char *str, int len )
{
  if ( *str != 't' && *str != 'f' ) {
    /* Handle the majority of the cases and avoid unnecessary function call to
       compare the strings. */
    json_writer_write_strn( writer, str, len );
  }
  else if ( strncasecmp( str, "true", len ) == 0 ) {
    json_writer_write_boolean( writer, TRUE );
  }
  else if ( strncasecmp( str, "false", len ) == 0 ) {
    json_writer_write_boolean( writer, FALSE );
  }
  else {
    json_writer_write_strn( writer, str, len );
  }
}

static void write_xml_str( json_writer_t *writer, const char *src  )
{
  write_xml_strn( writer, src, strlen( src ) );
}


typedef struct xml_converter_t {
  json_writer_t *writer;
  int skip_root;
  str_buf_t *str_buf;
  int first_elem;
  int status;
}xml_converter_t;

static void start_handler( void *converter_ptr, const char *name,
                           const char **atts )
{
  xml_converter_t *converter = converter_ptr;
  json_writer_t *writer = converter->writer;
  str_buf_t *str_buf = converter->str_buf;
  json_writer_ctx_t *context = json_writer_get_context( writer );
  json_writer_ctx_state state = json_writer_ctx_get_state( context );

  if ( str_buf->data_len > 0 &&
       !str_is_whitespace( str_buf->data, str_buf->data_len ) ) {
    converter->status = FALSE;
    fprintf( stderr, "Error: mixed content found in %s\ncontent:\n%.*s",
             json_writer_ctx_get_prop( context ), str_buf->data_len,
             str_buf->data );
  }

  STR_BUF_CLEAR( str_buf );

  if ( !converter->first_elem || !converter->skip_root ) {
    if ( state == JSON_INITIAL || state == JSON_PROPERTY ) {
      json_writer_start_object( writer );
    }
    json_writer_start_property( writer, name );
  }

  converter->first_elem = FALSE;

  if ( *atts ) {
    json_writer_start_object( writer );
    while ( *atts ) {
      json_writer_start_property( writer, *atts++ );
      write_xml_str( writer, *atts++ );
      json_writer_end_property( writer );
    }
  }
}

static void end_handler( void *converter_ptr, const char *name )
{
  xml_converter_t *converter = converter_ptr;
  json_writer_t *writer = converter->writer;
  str_buf_t *str_buf = converter->str_buf;
  json_writer_ctx_t *context = json_writer_get_context( writer );
  json_writer_ctx_state state = json_writer_ctx_get_state( context );

  if ( state == JSON_PROPERTY ) {
    if ( str_buf->data_len > 0 ) {
      write_xml_strn( writer, str_buf->data, str_buf->data_len );
    }
    else {
      json_writer_write_null( writer );
    }
    STR_BUF_CLEAR( str_buf );
  }
  else if ( state == JSON_IN_OBJECT ) {
    json_writer_end_object( writer );
  }
  else if ( state == JSON_IN_ARRAY ) {
    json_writer_end_array( writer );
  }

  if ( state != JSON_INITIAL ) {
    json_writer_end_property( writer );
  }
}

static void cdata_handler( void *converter_ptr, const char *data, int len )
{
  xml_converter_t *converter = converter_ptr;

  str_buf_write( converter->str_buf, data, len );
}

int xml_to_json( apr_pool_t *mp, apr_file_t *xml_file, int skip_root,
                 json_t **json )
{
  xml_converter_t converter;
  json_writer_t *writer;
  XML_Parser xp;
  apr_pool_t *tmp_mp;
  apr_status_t read_val;
  int xml_stat;
  char buffer[4096];
  apr_size_t len;
  int status;

  apr_pool_create( &tmp_mp, NULL );
  writer = json_writer_create( tmp_mp, mp );
  converter.writer = writer;
  converter.skip_root = skip_root;
  converter.str_buf = str_buf_create( tmp_mp, 4096 );
  converter.first_elem = TRUE;
  converter.status = TRUE;

  xp = XML_ParserCreate( NULL );
  XML_SetUserData( xp, &converter );
  XML_SetElementHandler( xp, start_handler, end_handler );
  XML_SetCharacterDataHandler( xp, cdata_handler );

  do {
    len = sizeof(buffer);
    read_val = apr_file_read( xml_file, buffer, &len );
    xml_stat = XML_Parse( xp, buffer, len, 0 );
  }while ( ( read_val == APR_SUCCESS ) && ( xml_stat == XML_STATUS_OK ) );

  *json = writer->json;
  apr_pool_destroy( tmp_mp );
  XML_ParserFree( xp );

  status = ( ( xml_stat == XML_STATUS_OK ) && ( converter.status == TRUE ) );

  return status;
}
