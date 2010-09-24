%{
#include <stdarg.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

/*
 * Define YY_DECL before including jxtl_lex.h so that it knows we are doing a
 * custom declaration of jxtl_lex.
 */
#define YY_DECL

#include "apr_macros.h"
#include "json.h"
#include "jxtl_parse.h"
#include "jxtl_lex.h"
#include "jxtl.h"
#include "parser.h"

#define callbacks ((jxtl_callback_t *) callbacks_ptr)

void jxtl_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                 void *callbacks_ptr, const char *error_string, ... );
%}

%name-prefix="jxtl_"
%defines
%verbose
%locations
%error-verbose
%pure-parser

%parse-param { yyscan_t scanner }
%parse-param { parser_t *parser }
%parse-param { void *callbacks_ptr }
%lex-param { yyscan_t scanner }

%union {
  int ival;
  unsigned char *string;
}

%token T_DIRECTIVE_START "{{" T_DIRECTIVE_END "}}"
       T_SECTION "section" T_SEPARATOR "separator"
       T_END "end" T_IF "if" T_ELSEIF "elseif" T_ELSE "else"
%token <string> T_TEXT "text"  T_PATH_EXPR "path expression" T_STRING "string"

%left T_ELSEIF T_ELSE

%%

document
  : text
;

text
  : /* empty */
  | text T_TEXT 
    {
      callbacks->text_handler( callbacks->user_data, $<string>2 );
    }
  | text value_directive
  | text section_directive
  | text if_directive
;

value_directive
  : T_DIRECTIVE_START T_PATH_EXPR T_DIRECTIVE_END
    {
      if ( !callbacks->value_handler( callbacks->user_data, $<string>2 ) ) {
        jxtl_error( &@2, scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
;

section_directive
  : T_DIRECTIVE_START T_SECTION T_PATH_EXPR
    {
      if ( !callbacks->section_start_handler( callbacks->user_data,
                                              $<string>3 ) ) {
        jxtl_error( &@2, scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    options
    T_DIRECTIVE_END
    section_content
    T_DIRECTIVE_START T_END T_DIRECTIVE_END
    { callbacks->section_end_handler( callbacks->user_data ); }
;

if_directive
  : T_DIRECTIVE_START T_IF T_PATH_EXPR T_DIRECTIVE_END
    {
      if ( !callbacks->if_start_handler( callbacks->user_data, $<string>3 ) ) {
        jxtl_error( &@3, scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    section_content
    rest_of_if

;

rest_of_if
  : T_DIRECTIVE_START T_ELSEIF T_PATH_EXPR T_DIRECTIVE_END
    {
      if ( !callbacks->elseif_handler( callbacks->user_data, $<string>3 ) ) {
        jxtl_error( &@3, scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    section_content rest_of_if
  | T_DIRECTIVE_START T_ELSE T_DIRECTIVE_END
    {
      callbacks->else_handler( callbacks->user_data );
    }
    section_content endif
  | endif
  ;

endif
  : T_DIRECTIVE_START T_END T_DIRECTIVE_END
    {
      callbacks->if_end_handler( callbacks->user_data );
    }

section_content
  : /* empty */
  | section_content T_TEXT
    {
      callbacks->text_handler( callbacks->user_data, $<string>2 );
    }
  | section_content value_directive
  | section_content section_directive
  | section_content if_directive
;

options
  : /* empty */
  | option
  | options ',' option
;

option
  : T_SEPARATOR '=' T_STRING
    {
      callbacks->separator_start_handler( callbacks->user_data );
      callbacks->text_handler( callbacks->user_data, $<string>3 );
      callbacks->separator_end_handler( callbacks->user_data );
    }
;

%%

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
  /** Reusable parser. */
  parser_t *jxtl_path_parser;
} jxtl_data_t;

void jxtl_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                 void *callbacks_ptr, const char *error_string, ... )
{
  va_list args;
  fprintf( stderr, "%s: %d.%d-%d.%d ", parser->get_filename( parser ),
	   yylloc->first_line, yylloc->first_column, yylloc->last_line,
	   yylloc->last_column );
  va_start( args, error_string );
  vfprintf( stderr, error_string, args );
  fprintf( stderr, "\n" );
  va_end( args );
}

/*
 * Convenience function to create a new content object and it on the current
 * array.
 */
static void jxtl_content_push( jxtl_data_t *data, jxtl_content_type type,
                               void *value )
{
  jxtl_content_t *content = NULL;

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
static void jxtl_text_func( void *user_data, unsigned char *text )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_content_push( data, JXTL_TEXT, apr_pstrdup( data->mp, (char *) text ) );
}

static int jxtl_section_start( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_section_t *section;

  section = apr_palloc( data->mp, sizeof( jxtl_section_t ) );
  jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr, &section->expr );
  section->content = apr_array_make( data->mp, 1024,
                                     sizeof( jxtl_content_t * ) );
  section->separator = apr_array_make( data->mp, 1024,
                                       sizeof( jxtl_content_t * ) );
  jxtl_content_push( data, JXTL_SECTION, section );
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = section->content;

  return ( data->jxtl_path_parser->parse_result == APR_SUCCESS ) ? TRUE : FALSE;
}

/**
 * Parser callback for when a section ends.
 * @param user_data The jxtl_data.
 * @param name The name of the section.
 */
static void jxtl_section_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;

  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
}

static int jxtl_if_start( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_if_t *jxtl_if;
  apr_array_header_t *if_block;

  if_block = apr_array_make( data->mp, 8, sizeof( jxtl_if_t * ) );
  jxtl_if = apr_palloc( data->mp, sizeof( jxtl_if_t ) );
  jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr, &jxtl_if->expr );
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof( jxtl_content_t * ) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  jxtl_content_push( data, JXTL_IF, if_block );
  
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = jxtl_if->content;

  return ( data->jxtl_path_parser->parse_result == APR_SUCCESS ) ? TRUE : FALSE;
}

static int jxtl_elseif( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  apr_array_header_t *content_array, *if_block;
  jxtl_if_t *jxtl_if;
  jxtl_content_t *content;

  content_array = APR_ARRAY_TAIL( data->content_array, apr_array_header_t * );
  content = APR_ARRAY_TAIL( content_array, jxtl_content_t * );
  if_block = (apr_array_header_t *) content->value;
  jxtl_if = apr_palloc( data->mp, sizeof( jxtl_if_t ) );
  jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr, &jxtl_if->expr );
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof( jxtl_content_t * ) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  data->current_array = jxtl_if->content;

  return ( data->jxtl_path_parser->parse_result == APR_SUCCESS ) ? TRUE : FALSE;
}

static void jxtl_else( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
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

static void jxtl_if_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  
  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
}

/**
 * Parser callback for when it encounters a separator directive.  All this does
 * is take the current section and set its current_array to the separator.
 * @param user_data The jxtl_data.
 */
static void jxtl_separator_start( void *user_data )
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
static void jxtl_separator_end( void *user_data )
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
static int jxtl_value_func( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_path_expr_t *path_expr;

  jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr, &path_expr );
  jxtl_content_push( data, JXTL_VALUE, path_expr );

  return ( data->jxtl_path_parser->parse_result == APR_SUCCESS ) ? TRUE : FALSE;
}

static char *jxtl_get_error( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  return parser_get_error( data->jxtl_path_parser );
}

parser_t *jxtl_parser_create( apr_pool_t *mp )
{
  parser_t *parser = parser_create( mp,
				    jxtl_lex_init,
				    jxtl_set_extra,
				    jxtl_lex_destroy,
				    jxtl__scan_buffer,
				    jxtl__delete_buffer,
				    jxtl_parse );
  jxtl_data_t *jxtl_data = apr_palloc( mp, sizeof(jxtl_data_t) );

  apr_pool_create( &jxtl_data->mp, mp );
  jxtl_data->mp = mp;
  jxtl_data->json = NULL;
  jxtl_data->content_array = apr_array_make( mp, 32,
                                            sizeof(apr_array_header_t *) );
  jxtl_data->current_array = apr_array_make( mp, 1024,
                                            sizeof(apr_array_header_t *) );
  jxtl_data->jxtl_path_parser = jxtl_path_parser_create( mp );

  jxtl_callback_t *jxtl_callbacks = apr_palloc( mp, sizeof(jxtl_callback_t) );
  jxtl_callbacks->text_handler = jxtl_text_func;
  jxtl_callbacks->section_start_handler = jxtl_section_start;
  jxtl_callbacks->section_end_handler = jxtl_section_end;
  jxtl_callbacks->if_start_handler = jxtl_if_start;
  jxtl_callbacks->elseif_handler = jxtl_elseif;
  jxtl_callbacks->else_handler = jxtl_else;
  jxtl_callbacks->if_end_handler = jxtl_if_end;
  jxtl_callbacks->separator_start_handler = jxtl_separator_start;
  jxtl_callbacks->separator_end_handler = jxtl_separator_end;
  jxtl_callbacks->value_handler = jxtl_value_func;
  jxtl_callbacks->user_data = jxtl_data;
  jxtl_callbacks->get_error_func = jxtl_get_error;
    
  parser_set_user_data( parser, jxtl_callbacks );

  return parser;
}

int jxtl_parser_parse_file( parser_t *parser, const char *file,
                            apr_array_header_t **content_array )
{
  jxtl_callback_t *jxtl_callbacks = parser_get_user_data( parser );
  jxtl_data_t *jxtl_data = jxtl_callbacks->user_data;
  apr_array_header_t *content;

  *content_array = NULL;
  APR_ARRAY_CLEAR( jxtl_data->content_array );
  APR_ARRAY_CLEAR( jxtl_data->current_array );

  content = apr_array_make( parser->mp, 1024, sizeof(jxtl_content_t *) );
  APR_ARRAY_PUSH( jxtl_data->current_array, apr_array_header_t * ) = content;

  if ( parser_parse_file( parser, file ) == 0 ) {
    *content_array = jxtl_data->current_array;
  }

  return parser->parse_result;
}
