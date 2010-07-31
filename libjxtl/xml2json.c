#include <apr_lib.h>
#include <apr_pools.h>
#include <apr_xml.h>

#include "json_node.h"
#include "json_writer.h"
#include "xml2json.h"

int xml_text_is_whitespace( apr_text *t )
{
  int len;
  int i;

  for ( ; t; t = t->next ) {
    len = strlen( t->text );
    for ( i = 0; i < len ; i++ ) {
      if ( !apr_isspace( t->text[i] ) ) {
	return 0;
      }
    }
  }

  return 1;
}

int xml_node_is_whitespace( apr_xml_elem *elem )
{
  apr_text *t;
  apr_xml_elem *child;
  
  t = elem->first_cdata.first;

  if ( !xml_text_is_whitespace( t ) ) {
    return 0;
  }

  for ( child = elem->first_child; child; child = child->next ) {
    t = child->following_cdata.first;
    if ( !xml_text_is_whitespace( t ) ) {
      return 0;
    }
  }

  return 1;
}

static json_type xml_node_type( apr_xml_elem *node )
{
  /* Empty element is null */
  if ( APR_XML_ELEM_IS_EMPTY( node ) && !node->attr )
    return JSON_NULL;
  
  /* Not empty and has attrs, it must be an object. */
  if ( node->attr )
    return JSON_OBJECT;

  /* Not empty, does not have attrs, and does not have a first child.  This
   * must be a text node.
   */
  if ( !node->first_child ) {
    return JSON_STRING;
  }

  /* Not empty, does not have attrs, and has a first child.  If it has text,
   * verify that it is all white space.  If it's not, that means it has mixed
   * content and all the content of this node will be treated as a
   * string.
   */
  if ( node->first_cdata.first ) {
    if ( !xml_node_is_whitespace( node ) ) {
      return JSON_STRING;
    }
    else {
      return JSON_OBJECT;
    }
  }

  /* Not empty, does not have attrs, has a first child, and has no text. */
  return JSON_OBJECT;
}

static void xml_node_process( apr_pool_t *mp, apr_xml_elem *root,
			      json_writer_t *writer )
{
  apr_xml_elem *node;
  apr_xml_attr *attr;
  json_type type;
  const char *elem_buf;
  apr_size_t buf_size;

  if ( !root )
    return;

  for ( node = root; node; node = node->next ) {
    type = xml_node_type( node );
    json_writer_property_start( writer, (unsigned char *) node->name );
      
    if ( type == JSON_NULL ) {
      json_writer_null_write( writer );
    }
    else if ( type == JSON_STRING ) {
      apr_xml_to_text( mp, node, APR_XML_X2T_INNER, NULL, NULL, &elem_buf,
		       &buf_size );
      json_writer_string_write( writer, (unsigned char *) elem_buf );
    }
    else if ( type == JSON_OBJECT ) {
      json_writer_object_start( writer );
        
      for ( attr = node->attr; attr; attr = attr->next ) {
	json_writer_property_start( writer, (unsigned char *) attr->name );
	json_writer_string_write( writer, (unsigned char *) attr->value );
	json_writer_property_end( writer );
      }

      xml_node_process( mp, node->first_child, writer );
      
      json_writer_object_end( writer );
    }
    json_writer_property_end( writer );
  }
}

int xml_file_read( const char *filename, json_writer_t *writer, int skip_root )
{
  apr_pool_t *mp;
  apr_xml_parser *parser;
  apr_xml_doc *doc;
  apr_xml_elem *elem;
  apr_file_t *file;

  apr_pool_create( &mp, NULL );
  apr_file_open( &file, filename, APR_READ | APR_BUFFERED, 0, mp );
  apr_xml_parse_file( mp, &parser, &doc, file, 4096 );

  elem = doc->root;
  if ( skip_root ) {
    elem = elem->first_child;
  }

  json_writer_object_start( writer );
  xml_node_process( mp, elem, writer );
  json_writer_object_end( writer );

  apr_file_close( file );
  apr_pool_destroy( mp );
  return 0;
}
