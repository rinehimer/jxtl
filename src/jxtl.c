#include <apr_getopt.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"

#include "json.h"
#include "jxtl_path.h"
#include "json_writer.h"

#include "jxtl.h"
#include "jxtl_parse.h"
#include "jxtl_lex.h"
#include "lex_extra.h"
#include "xml2json.h"

/** Constants used for calling the print function */
typedef enum section_print_type {
  PRINT_SECTION,
  PRINT_SEPARATOR
} section_print_type;

typedef enum jxtl_content_type {
  JXTL_TEXT,
  JXTL_SECTION,
  JXTL_VALUE,
  JXTL_IF
} jxtl_content_type;

typedef struct jxtl_if_t {
  jxtl_path_expr_t *expr;
  apr_array_header_t *content;
} jxtl_if_t;

typedef struct jxtl_section_t {
  /** Compiled path expression. */
  jxtl_path_expr_t *expr;
  /** Array of the content in the section. */
  apr_array_header_t *content;
  /** Array of content for the separator. */
  apr_array_header_t *separator;
} jxtl_section_t;

typedef struct jxtl_content_t {
  /** What this content contains in its value pointer. */
  jxtl_content_type type;
  /** A string, pointer to a jxtl_section_t, jxtl_if_t or a jxtl_path_expr_t */
  void *value;
} jxtl_content_t;

/**
 * Structure to hold data during parsing.  One of these will be passed to the
 * callback functions.
 */
typedef struct jxtl_data_t {
  /** Memory pool */
  apr_pool_t *mp;
  /** Pointer to the JSON object */
  json_t *json;
  /** Pointer to the current content array. */
  apr_array_header_t *current_array;
  /** Array of content arrays. */
  apr_array_header_t *content_array;
  /** Reusable path builder. */
  jxtl_path_builder_t path_builder;
} jxtl_data_t;

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

static void jxtl_section_print( jxtl_section_t *section,
                                json_t *json,
                                section_print_type print_type );

static void jxtl_content_print( apr_array_header_t *content_array,
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

  path_obj = jxtl_path_obj_create( NULL );

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
      jxtl_section_print( tmp_section, json, PRINT_SECTION );
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
	     jxtl_path_compiled_eval( jxtl_if->expr, json, path_obj ) ) {
          jxtl_content_print( jxtl_if->content, json, print_type );
          break;
        }
      }
      break;

    case JXTL_VALUE:
      if ( jxtl_path_compiled_eval( content->value, json, path_obj ) ) {
	json_value = APR_ARRAY_IDX( path_obj->nodes, 0, json_t * );
	json_value_print( json_value );
      }
      break;
    }
    prev_content = content;
  }

  jxtl_path_obj_destroy( path_obj );
}

/**
 * Print a saved section
 */
static void jxtl_section_print( jxtl_section_t *section,
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

  path_obj = jxtl_path_obj_create( NULL );

  num_items = jxtl_path_compiled_eval( section->expr, json, path_obj );
  num_printed = 0;
  for ( i = 0; i < path_obj->nodes->nelts; i++ ) {
    json_value = APR_ARRAY_IDX( path_obj->nodes, i, json_t * );
    jxtl_content_print( section->content, json_value, PRINT_SECTION );
    num_printed++;
    /* Only print the separator if it's not the last one */
    if ( num_printed < num_items )
      jxtl_content_print( section->separator, json_value, PRINT_SEPARATOR );
  }

  jxtl_path_obj_destroy( path_obj );
}

/*
 * Convenience function to create a new content object and it on the current
 * array.
 */
static void jxtl_content_push( jxtl_data_t *data, jxtl_content_type type,
                               void *value )
{
  jxtl_content_t *content = NULL;
  jxtl_section_t *section = NULL;
  apr_array_header_t *current_content_array;

  content = apr_palloc( data->mp, sizeof( jxtl_content_t ) );
  content->type = type;
  content->value = value;

  APR_ARRAY_PUSH( data->current_array, jxtl_content_t * ) = content;
}

/*****************************************************************************
 * Parser callbacks
 *****************************************************************************/

/**
 * Parser callback for when it finds text.
 * @param user_data The jxtl_data.
 * @param text The text.
 */
void jxtl_text_func( void *user_data, unsigned char *text )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;

  if ( !apr_is_empty_array( data->content_array ) ) {
    /* Save off the text if we are nested. */
    jxtl_content_push( data, JXTL_TEXT,
                       apr_pstrdup( data->mp, (char *) text ) );
  }
  else {
    /* Not inside any sections print the text. */
    printf( "%s", text );
  }
}

void jxtl_section_start( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_section_t *section;

  section = apr_palloc( data->mp, sizeof( jxtl_section_t ) );
  section->expr = jxtl_path_compile( &data->path_builder, expr );
  section->content = apr_array_make( data->mp, 1024,
				     sizeof( jxtl_content_t * ) );
  section->separator = apr_array_make( data->mp, 1024,
                                       sizeof( jxtl_content_t * ) );
  jxtl_content_push( data, JXTL_SECTION, section );
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = section->content;
}

/**
 * Parser callback for when a section ends.
 * @param user_data The jxtl_data.
 * @param name The name of the section.
 */
void jxtl_section_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_section_t *section;

  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
  
  if ( apr_is_empty_array( data->content_array ) ) {
    /* Process saved document fragment */
    jxtl_content_print( data->current_array, data->json, PRINT_SECTION );
    /* Clear the pool when we finish a section. */
    APR_ARRAY_CLEAR( data->current_array );
    apr_pool_clear( data->mp );
  }
}

void jxtl_if_start( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_if_t *jxtl_if;
  apr_array_header_t *if_block;

  if_block = apr_array_make( data->mp, 8, sizeof( jxtl_if_t * ) );
  jxtl_if = apr_palloc( data->mp, sizeof( jxtl_if_t ) );
  jxtl_if->expr = jxtl_path_compile( &data->path_builder, expr );
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof( jxtl_content_t * ) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  jxtl_content_push( data, JXTL_IF, if_block );
  
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = jxtl_if->content;
}

void jxtl_elseif( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  apr_array_header_t *content_array, *if_block;
  jxtl_if_t *jxtl_if;
  jxtl_content_t *content;

  content_array = APR_ARRAY_TAIL( data->content_array, apr_array_header_t * );
  content = APR_ARRAY_TAIL( content_array, jxtl_content_t * );
  if_block = (apr_array_header_t *) content->value;
  jxtl_if = apr_palloc( data->mp, sizeof( jxtl_if_t ) );
  jxtl_if->expr = jxtl_path_compile( &data->path_builder, expr );
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof( jxtl_content_t * ) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  data->current_array = jxtl_if->content;
}

void jxtl_else( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_section_t *section;
  apr_array_header_t *content_array, *if_block;
  jxtl_if_t *jxtl_if;
  jxtl_content_t *content;
  
  content_array = APR_ARRAY_TAIL( data->content_array, apr_array_header_t * );
  content = APR_ARRAY_TAIL( content_array, jxtl_content_t * );
  if_block = (apr_array_header_t *) content->value;

  jxtl_if = apr_palloc( data->mp, sizeof( jxtl_if_t ) );
  jxtl_if->expr = NULL;
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof( jxtl_content_t * ) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  data->current_array = jxtl_if->content;
}

void jxtl_if_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_section_t *section;
  
  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
  if ( apr_is_empty_array( data->content_array ) ) {
    jxtl_content_print( data->current_array, data->json, PRINT_SECTION );
    APR_ARRAY_CLEAR( data->current_array );
    apr_pool_clear( data->mp );
  }
}

/**
 * Parser callback for when it encounters a separator directive.  All this does
 * is take the current section and set its current_array to the separator.
 * @param user_data The jxtl_data.
 */
void jxtl_separator_start( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  apr_array_header_t *content_array;
  jxtl_content_t *content;
  jxtl_section_t *section;

  content_array = APR_ARRAY_TAIL( data->content_array, apr_array_header_t * );

  content = APR_ARRAY_TAIL( content_array, jxtl_content_t * );
  section = content->value;
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = section->separator;
}

/**
 * Parser callback for when a separator directive is ended.  Just sets the
 * current_array of the section back to the content.
 * @param user_data The jxtl_data.
 */
void jxtl_separator_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
}

/**
 * Parser callback function for when it encounters a value reference in the
 * template, i.e. {{value}}.  If we are not nested at all, it is printed
 * immediately.  Otherwise, the name is just saved off for later processing.
 * @param user_data The jxtl_data.
 * @param name The name of the value to lookup.
 */
void jxtl_value_func( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  json_t *json_value;

  if ( !apr_is_empty_array( data->content_array ) ) {
    jxtl_content_push( data, JXTL_VALUE,
		       jxtl_path_compile( &data->path_builder, expr ) );
  }
  else {
    jxtl_path_obj_t *path_obj;
    path_obj = jxtl_path_obj_create( data->mp );
    jxtl_path_eval( expr, data->json, path_obj );
    json_value = APR_ARRAY_IDX( path_obj->nodes, 0, json_t * );
    json_value_print( json_value );
    jxtl_path_obj_destroy( path_obj );
    apr_pool_clear( data->mp );
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
int jxtl_load_data( const char *json_file, const char *xml_file,
		    int skip_root, json_writer_t *writer )
{
  int ret = 1;

  if ( xml_file ) {
    ret = xml_file_read( xml_file, writer, skip_root );
  }
  else if ( json_file ) {
    ret = json_file_parse( json_file, writer );
  }

  return ret;
}

int main( int argc, char const * const *argv )
{
  apr_pool_t *mp;
  int parse_result;
  lex_extra_t lex_extra;
  yyscan_t jxtl_scanner;
  jxtl_data_t jxtl_data;
  const char *template_file = NULL;
  const char *json_file = NULL;
  const char *xml_file = NULL;
  int skip_root;
  json_writer_t writer;

  apr_app_initialize( NULL, NULL, NULL );
  apr_pool_create( &mp, NULL );

  jxtl_init( argc, argv, mp, &template_file, &json_file, &xml_file,
             &skip_root );

  /*
   * Setup our callback object.  Note that the stack is created using this
   * memory pool, and not its internal one.  The reason is then when we are
   * done processing sections, we want to be able to clear all of that memory
   * quickly and leave the section array allocated.
   */
  apr_pool_create( &jxtl_data.mp, NULL );
  jxtl_data.json = NULL;
  jxtl_data.content_array = apr_array_make( mp, 32,
                                            sizeof(apr_array_header_t *) );
  jxtl_data.current_array = apr_array_make( mp, 1024,
                                            sizeof(jxtl_section_t *) );

  jxtl_callback_t callbacks = {
    jxtl_text_func,
    jxtl_section_start,
    jxtl_section_end,
    jxtl_if_start,
    jxtl_elseif,
    jxtl_else,
    jxtl_if_end,
    jxtl_separator_start,
    jxtl_separator_end,
    jxtl_value_func,
    &jxtl_data
  };

  json_writer_init( &writer );
  jxtl_path_builder_init( &jxtl_data.path_builder );

  if ( jxtl_load_data( json_file, xml_file, skip_root, &writer ) == 0 ) {
    jxtl_data.json = writer.json;
    jxtl_lex_init( &jxtl_scanner );
    lex_extra_init( &lex_extra, template_file );
    jxtl_set_extra( &lex_extra, jxtl_scanner );
    parse_result = jxtl_parse( jxtl_scanner, &callbacks );
    lex_extra_destroy( &lex_extra );
    jxtl_lex_destroy( jxtl_scanner );
  }

  jxtl_path_builder_destroy( &jxtl_data.path_builder );
  json_writer_destroy( &writer );

  apr_pool_destroy( jxtl_data.mp );
  apr_pool_destroy( mp );
  apr_terminate();
  return parse_result;
}
