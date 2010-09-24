#include <apr_general.h>
#include <apr_lib.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_xml.h>

#include "json.h"
#include "json_writer.h"
#include "xml2json.h"

static int text_is_whitespace( apr_text *t )
{
  const char *c;

  for ( ; t; t = t->next ) {
    for ( c = t->text; *c; c++ ) {
      if ( !apr_isspace( *c ) ) {
	return FALSE;
      }
    }
  }

  return TRUE;
}

/**
 * Determine if an elem contains only whitespace text nodes.
 */
static int xml_elem_is_whitespace( apr_xml_elem *elem )
{
  apr_text *t;
  apr_xml_elem *child;
  
  t = elem->first_cdata.first;

  if ( !text_is_whitespace( t ) ) {
    return FALSE;
  }

  for ( child = elem->first_child; child; child = child->next ) {
    t = child->following_cdata.first;
    if ( !text_is_whitespace( t ) ) {
      return FALSE;
    }
  }

  return TRUE;
}

/**
 * Determine a JSON type for given XML elem.
 */
static json_type xml_elem_type( apr_xml_elem *elem )
{
  /* Empty element is null */
  if ( APR_XML_ELEM_IS_EMPTY( elem ) && !elem->attr ) {
    return JSON_NULL;
  }
  
  /* Not empty and has attrs, it must be an object. */
  if ( elem->attr ) {
    return JSON_OBJECT;
  }

  /* Not empty, does not have attrs, and does not have a first child.  This
   * must be a text elem.
   */
  if ( !elem->first_child ) {
    return JSON_STRING;
  }

  /* Not empty, does not have attrs, and has a first child.  If it has text,
   * verify that it is all white space.  If it's not, that means it has mixed
   * content and all the content of this elem will be treated as a
   * string.
   */
  if ( elem->first_cdata.first ) {
    if ( !xml_elem_is_whitespace( elem ) ) {
      return JSON_STRING;
    }
    else {
      return JSON_OBJECT;
    }
  }

  /* Not empty, does not have attrs, has a first child, and has no text. */
  return JSON_OBJECT;
}

/**
 * When we have a string to write, check to see if it is a valid boolean value
 * in JSON.  It's possible we may do number checking here in the future.
 */
static void write_xml_string( json_writer_t *writer, const char *str  )
{
  if ( *str != 't' && *str != 'f' ) {
    /* Handle the majority of the cases and avoid unnecessary function call to
       compare the strings. */
    json_writer_write_string( writer, (unsigned char *) str );
  }
  else if ( apr_strnatcmp( str, "true" ) == 0 ) {
    json_writer_write_boolean( writer, TRUE );
  }
  else if ( apr_strnatcmp( str, "false" ) == 0 ) {
    json_writer_write_boolean( writer, FALSE );
  }
  else {
    json_writer_write_string( writer, (unsigned char *) str );
  }
}

static void xml_elem_to_json( apr_pool_t *mp, apr_xml_elem *root,
			      json_writer_t *writer )
{
  apr_xml_elem *elem;
  apr_xml_attr *attr;
  json_type type;
  const char *elem_buf;
  apr_size_t buf_size;

  if ( !root )
    return;

  for ( elem = root; elem; elem = elem->next ) {
    type = xml_elem_type( elem );
    json_writer_start_property( writer, (unsigned char *) elem->name );
      
    if ( type == JSON_NULL ) {
      json_writer_write_null( writer );
    }
    else if ( type == JSON_STRING ) {
      apr_xml_to_text( mp, elem, APR_XML_X2T_INNER, NULL, NULL, &elem_buf,
		       &buf_size );
      write_xml_string( writer, elem_buf );
    }
    else if ( type == JSON_OBJECT ) {
      json_writer_start_object( writer );
        
      for ( attr = elem->attr; attr; attr = attr->next ) {
	json_writer_start_property( writer, (unsigned char *) attr->name );
	write_xml_string( writer, attr->value );
	json_writer_end_property( writer );
      }

      xml_elem_to_json( mp, elem->first_child, writer );
      
      json_writer_end_object( writer );
    }
    json_writer_end_property( writer );
  }
}

int xml_file_to_json( const char *filename, json_writer_t *writer,
		      int skip_root )
{
  apr_pool_t *xml_mp;
  apr_xml_parser *parser;
  apr_xml_doc *doc;
  apr_xml_elem *elem;
  apr_file_t *file;
  apr_status_t status;
  int is_stdin;

  is_stdin = ( filename && apr_strnatcasecmp( filename, "-" ) == 0 );

  apr_pool_create( &xml_mp, NULL );

  if ( is_stdin ) {
    status = apr_file_open_stdin( &file, xml_mp );
  }
  else {
    status = apr_file_open( &file, filename, APR_READ | APR_BUFFERED, 0,
			    xml_mp );
  }

  if ( status == APR_SUCCESS ) {
    status = apr_xml_parse_file( xml_mp, &parser, &doc, file, 4096 );
  }

  if ( status == APR_SUCCESS ) {
    elem = doc->root;
    if ( skip_root ) {
      elem = elem->first_child;
    }
    
    json_writer_start_object( writer );
    xml_elem_to_json( xml_mp, elem, writer );
    json_writer_end_object( writer );
  }

  apr_file_close( file );
  apr_pool_destroy( xml_mp );

  return status;
}
