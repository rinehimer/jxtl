#include <stdio.h>
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_tables.h>

#include <libxml/xmlwriter.h>

#include "apr_macros.h"

#include "json_node.h"
#include "json.h"

#include "json_parse.h"
#include "json_lex.h"
#include "json_writer.h"
#include "lex_extra.h"

static void print_spaces( int num )
{
  while ( num-- > 0 ) {
    printf( " " );
  }
}

static void json_string_print( unsigned char *str )
{
  unsigned char *c = str;

  printf( "\"" );
  while ( *c ) {
    if ( *c < 32 ) {
      printf( "\\" );
      switch ( *c ) {
      case '\b':
	printf( "b" );
	break;
      case '\t':
	printf( "t" );
	break;
      case '\n':
	printf( "n" );
	break;
      case '\f':
	printf( "f" );
	break;
      case '\r':
	printf( "r" );
	break;
      default:
	printf( "u%.4x", *c );
	break;
      }
    }
    else if ( *c == '\\' ) {
      printf( "\\\\" );
    }
    else if ( *c == '"' ) {
      printf( "\\\"" );
    }
    else
      printf( "%c", *c );
    c++;
  }
  printf( "\"" );
}

static void json_object_print_internal( json_t *json,
					int first,
					int depth,
					int indent )
{
  apr_array_header_t *arr = NULL;
  int i = 0;
  int len;
  json_t *tmp_json = NULL;
  unsigned char c;
  apr_hash_index_t *idx;

  if ( !first )
    printf( "," );

  if ( ( depth > 0 ) && indent ) {
    printf( "\n" );
    print_spaces( depth * indent );
  }

  if ( JSON_NAME( json ) ) {
    json_string_print( JSON_NAME( json ) );
    printf( ":" );
     if ( indent )
      print_spaces( 1 );
  }

  switch ( json->type ) {
  case JSON_STRING:
    json_string_print( json->value.string );
    break;

  case JSON_INTEGER:
    printf( "%d", json->value.integer );
    break;

  case JSON_NUMBER:
    printf( "%g", json->value.number );
    break;

  case JSON_OBJECT:
    printf( "{" );
    for ( i = 0, idx = apr_hash_first( NULL, json->value.object ); idx;
	  i++, idx = apr_hash_next( idx ) ) {
      apr_hash_this( idx, NULL, NULL, (void **) &tmp_json );
      json_object_print_internal( tmp_json, i == 0, depth + 1, indent );
    }
    
    if ( indent && i > 0 ) {
      printf( "\n" );
      print_spaces( depth * indent );
    }
    printf( "}" );
    break;

  case JSON_ARRAY:
    arr = json->value.array;
    printf( "[" );
    for ( i = 0; arr && i < arr->nelts; i++ ) {
      tmp_json = APR_ARRAY_IDX( arr, i, json_t * );
      json_object_print_internal( tmp_json, i == 0, depth + 1, indent );
    }

    if ( indent && i > 0 ) {
      printf( "\n" );
      print_spaces( depth * indent );
    }

    printf( "]" );
    break;
    
  case JSON_BOOLEAN:
    ( json->value.boolean ) ? printf( "true" ) :
                              printf( "false" );
    break;
    
  case JSON_NULL:
    printf( "null" );
    break;
    
  default:
    fprintf( stderr, "error:  unrecognized object type\n" );
    break;
  }

  if ( ( depth == 0 ) && indent ) {
    printf( "\n" );
  }
}

/*
 * Externally visible function that invokes the internal print function.
 */
void json_object_print( json_t *json, int indent )
{
  json_object_print_internal( json, 1, 0, indent );
}



static void json_write_string( xmlTextWriterPtr xml_writer,
			       unsigned char *str )
{
  unsigned char *c = str;

  while ( *c ) {
    if ( ( *c < 0x20 ) && ( *c != 0x9 ) && ( *c != 0xa ) && ( *c != 0xd ) ) {
      /*
       * XML can't handle these characters, so just store as the Unicode escape
       * sequence.
       */
      xmlTextWriterWriteFormatString( xml_writer, "\\u%.4x", *c );
    }
    else
      xmlTextWriterWriteFormatString( xml_writer, "%c", *c );
    c++;
  }
}

static void json_to_xml_internal( json_t *json,
				  xmlTextWriterPtr xml_writer )
{
  json_t *tmp_json;
  apr_array_header_t *arr;
  unsigned char *json_name = JSON_NAME( json );
  apr_hash_index_t *idx;
  int i;

  if ( json_name && json->type != JSON_ARRAY ) {
    xmlTextWriterStartElement( xml_writer, json_name );
  }

  switch ( json->type ) {
  case JSON_STRING:
    json_write_string( xml_writer, json->value.string );
    break;

  case JSON_INTEGER:
    xmlTextWriterWriteFormatString( xml_writer, "%d", json->value.integer );
    break;

  case JSON_NUMBER:
    xmlTextWriterWriteFormatString( xml_writer, "%lf", json->value.number );
    break;

  case JSON_OBJECT:
    for ( idx = apr_hash_first( NULL, json->value.object ); idx;
	  idx = apr_hash_next( idx ) ) {
      apr_hash_this( idx, NULL, NULL, (void **) &tmp_json );
      json_to_xml_internal( tmp_json, xml_writer );
    }

    break;

  case JSON_ARRAY:
    arr = json->value.array;

    for ( i = 0; i < arr->nelts; i++ ) {
      tmp_json = APR_ARRAY_IDX( arr, i, json_t * );
      xmlTextWriterStartElement( xml_writer, json_name );
      json_to_xml_internal( tmp_json, xml_writer );
      xmlTextWriterEndElement( xml_writer );
    }
    break;
    
  case JSON_BOOLEAN:
    ( json->value.boolean ) ?
      xmlTextWriterWriteString( xml_writer, BAD_CAST "true" ) :
      xmlTextWriterWriteString( xml_writer, BAD_CAST "false" );
    break;
    
  case JSON_NULL:
    xmlTextWriterWriteString( xml_writer, BAD_CAST "null" );
    break;

  default:
    fprintf( stderr, "error:  unrecognized object type\n" );
    break;
  }

  if ( json_name && json->type != JSON_ARRAY ) {
    xmlTextWriterEndElement( xml_writer );
  }
}

void json_to_xml( json_t *json, const char *filename, int indent )
{
  xmlTextWriterPtr xml_writer;

  xml_writer = xmlNewTextWriterFilename( filename, 0 );
  xmlTextWriterSetIndent( xml_writer, indent );

  xmlTextWriterStartDocument( xml_writer, NULL, "UTF-8", NULL );
  xmlTextWriterStartElement( xml_writer, BAD_CAST "json" );

  json_to_xml_internal( json, xml_writer );

  xmlTextWriterEndElement( xml_writer );
  xmlTextWriterEndDocument( xml_writer );

  xmlTextWriterFlush( xml_writer );
  xmlFreeTextWriter( xml_writer );
}
