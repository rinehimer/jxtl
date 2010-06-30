#include <libxml/parser.h>

#include "json_node.h"
#include "json_writer.h"
#include "xml2json.h"

static json_type xml_node_type( xmlNodePtr node )
{
  xmlNodePtr ptr;
  int num_text_nodes = 0;

  if ( !node->children && !node->properties )
    return JSON_NULL;

  if ( node->properties )
    return JSON_OBJECT;

  for ( ptr = node->children; ptr; ptr = ptr->next ) {
    if ( ( ptr->type == XML_TEXT_NODE ) ||
         ( ptr->type == XML_CDATA_SECTION_NODE ) )
      num_text_nodes++;
  }

  return ( num_text_nodes >= 1 ) ? JSON_STRING : JSON_OBJECT;
}

static void xml_node_process( xmlNodePtr root, json_writer_t *writer )
{
  xmlNodePtr node;
  xmlNodePtr tmp_node;
  xmlAttrPtr attr;
  json_type type;
  int need_text_prop = 0;
  xmlBufferPtr elem_buf;

  if ( !root )
    return;

  need_text_prop = ( root->parent && root->parent->properties );
  elem_buf = xmlBufferCreate();

  for ( node = root; node; node = node->next ) {
    switch ( node->type ) {
        
    case XML_TEXT_NODE:
    case XML_CDATA_SECTION_NODE:
      for ( tmp_node = node; tmp_node; tmp_node = tmp_node->next ) {
        xmlNodeBufGetContent( elem_buf, tmp_node );
      }
      if ( need_text_prop )
        json_writer_property_start( writer, (unsigned char *) "text" );
      json_writer_string_write( writer, elem_buf->content );
      if ( need_text_prop )
        json_writer_property_end( writer );
      xmlBufferFree( elem_buf );
      return;
      break;
      
    case XML_ELEMENT_NODE:
      type = xml_node_type( node );
      json_writer_property_start( writer, (unsigned char *) node->name );
      
      if ( type == JSON_NULL ) {
        json_writer_null_write( writer );
      }
      else if ( type == JSON_STRING ) {
        for ( tmp_node = node->children; tmp_node;
              tmp_node = tmp_node->next ) {
          if ( tmp_node->type == XML_TEXT_NODE ||
               tmp_node->type == XML_CDATA_SECTION_NODE ) {
            xmlNodeBufGetContent( elem_buf, tmp_node );
          }
          else {
            xmlNodeDump( elem_buf, tmp_node->doc, tmp_node, 0, 0 );
          }
        }
        json_writer_string_write( writer, elem_buf->content );
      }
      else if ( type == JSON_OBJECT ) {
        json_writer_object_start( writer );
        
        for ( attr = node->properties; attr; attr = attr->next ) {
          xmlNodeBufGetContent( elem_buf, (xmlNodePtr) attr );
          json_writer_property_start( writer, (unsigned char *) attr->name );
          json_writer_string_write( writer, elem_buf->content );
          json_writer_property_end( writer );
          xmlBufferEmpty( elem_buf );
        }

        xml_node_process( node->children, writer );
        
        json_writer_object_end( writer );
      }
      
      json_writer_property_end( writer );
      break;
    }
    xmlBufferEmpty( elem_buf );
  }

  xmlBufferFree( elem_buf );
}

int xml_file_read( const char *filename, json_writer_t *writer, int skip_root )
{
  xmlDocPtr doc;
  xmlNodePtr node;

  doc = xmlReadFile( filename, NULL, XML_PARSE_NOBLANKS );
  if ( !doc ) {
    fprintf( stderr, "%s : failed to parse\n", filename );
    return -1;
  }

  node = xmlDocGetRootElement( doc );
  if ( skip_root ) {
    node = node->children;
  }

  json_writer_object_start( writer );
  xml_node_process( node, writer );
  json_writer_object_end( writer );

  xmlFreeDoc( doc );
  xmlCleanupParser();
  return 0;
}
