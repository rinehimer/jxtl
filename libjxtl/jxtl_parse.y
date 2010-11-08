/*
 * $Id$
 *
 * Description
 *   Bison source file for generating the jxtl grammar.
 *
 * Copyright 2010 Dan Rinehimer
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

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
       T_SECTION "section" T_SEPARATOR "separator" T_FORMAT "format"
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
  : T_DIRECTIVE_START T_PATH_EXPR
    {
      if ( !callbacks->value_handler( callbacks->user_data, $<string>2 ) ) {
        jxtl_error( &@2, scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    options T_DIRECTIVE_END
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
    options T_DIRECTIVE_END section_content
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
  | T_FORMAT '=' T_STRING
    {
      callbacks->format_handler( callbacks->user_data, $<string>3 );
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
  /** Pointer to the last section or value pushed on. */
  jxtl_content_t *last_section_or_value;
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

/**
 * Allocate a new jxtl_data_t.
 */
static jxtl_data_t *jxtl_data_create( apr_pool_t *mp )
{
  jxtl_data_t *data = apr_palloc( mp, sizeof(jxtl_data_t) );

  data->mp = mp;
  data->json = NULL;
  data->content_array = apr_array_make( data->mp, 1024,
                                        sizeof(apr_array_header_t *) );
  data->current_array = NULL;
  data->jxtl_path_parser = jxtl_path_parser_create( mp );

  return data;
}

/**
 * Reset the data each time before parsing.  Basically we have to clear out
 * the content_array and create a new initial array.  Note that the memory
 * pool used is not cleared because it's not owned by us and clearing it could
 * clear the allocation of a previous template that was parsed using the same
 * parser.
 */
static void jxtl_data_reset( jxtl_data_t *data )
{
  apr_array_header_t *initial_array;
  APR_ARRAY_CLEAR( data->content_array );
  initial_array = apr_array_make( data->mp, 1024,
                                  sizeof(apr_array_header_t *) );
  APR_ARRAY_PUSH( data->content_array, apr_array_header_t * ) = initial_array;
  data->current_array = initial_array;
}

/*
 * Convenience function to create a new content object and it on the current
 * array.
 */
static void jxtl_content_push( jxtl_data_t *data, jxtl_content_type type,
                               void *value )
{
  jxtl_content_t *content = NULL;

  content = apr_palloc( data->mp, sizeof(jxtl_content_t) );
  content->type = type;
  content->value = value;
  content->separator = NULL;
  content->format = NULL;

  if ( ( type == JXTL_SECTION ) || ( type == JXTL_VALUE ) ) {
    data->last_section_or_value = content;
  }

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
  int result;

  section = apr_palloc( data->mp, sizeof(jxtl_section_t) );
  result = jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr,
                                          &section->expr );
  section->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  jxtl_content_push( data, JXTL_SECTION, section );
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = section->content;

  return result;
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
  int result;

  if_block = apr_array_make( data->mp, 8, sizeof(jxtl_if_t *) );
  jxtl_if = apr_palloc( data->mp, sizeof(jxtl_if_t) );
  result = jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr,
                                          &jxtl_if->expr );
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  jxtl_content_push( data, JXTL_IF, if_block );
  
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = jxtl_if->content;

  return result;
}

static int jxtl_elseif( void *user_data, unsigned char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  apr_array_header_t *content_array, *if_block;
  jxtl_if_t *jxtl_if;
  jxtl_content_t *content;
  int result;

  content_array = APR_ARRAY_TAIL( data->content_array, apr_array_header_t * );
  content = APR_ARRAY_TAIL( content_array, jxtl_content_t * );
  if_block = (apr_array_header_t *) content->value;
  jxtl_if = apr_palloc( data->mp, sizeof(jxtl_if_t) );
  result = jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr,
                                          &jxtl_if->expr );
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  data->current_array = jxtl_if->content;

  return result;
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

  jxtl_if = apr_palloc( data->mp, sizeof(jxtl_if_t) );
  jxtl_if->expr = NULL;
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
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
  jxtl_content_t *content = data->last_section_or_value;

  content->separator = apr_array_make( data->mp, 1, sizeof(jxtl_content_t *) );

  /*
   * Save off the current array and then make the current array the separator.
   */
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;
  data->current_array = content->separator;
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
  int result;

  result = jxtl_path_parser_parse_buffer( data->jxtl_path_parser, expr,
                                          &path_expr );
  jxtl_content_push( data, JXTL_VALUE, path_expr );

  return result;
}

static char *jxtl_get_error( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  return parser_get_error( data->jxtl_path_parser );
}

static void jxtl_format( void *user_data, char *format )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  data->last_section_or_value->format = apr_pstrdup( data->mp, format );
}

static jxtl_template_t *jxtl_template_create( apr_pool_t *mp,
                                              apr_array_header_t *content )
{
  jxtl_template_t *template;
  template = apr_palloc( mp, sizeof(jxtl_template_t) );
  template->content = content;
  template->format = NULL;

  return template;
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
  jxtl_callbacks->get_error_func = jxtl_get_error;
  jxtl_callbacks->format_handler = jxtl_format;
  jxtl_callbacks->user_data = jxtl_data_create( mp );
    
  parser_set_user_data( parser, jxtl_callbacks );

  return parser;
}

int jxtl_parser_parse_file( parser_t *parser, const char *file,
                            jxtl_template_t **template_ptr )
{
  jxtl_callback_t *jxtl_callbacks = parser_get_user_data( parser );
  jxtl_data_t *jxtl_data = (jxtl_data_t *) jxtl_callbacks->user_data;
  int result = FALSE;

  *template_ptr = NULL;
  jxtl_data_reset( jxtl_data );

  if ( parser_parse_file( parser, file ) ) {
    *template_ptr = jxtl_template_create( parser->mp,
                                          jxtl_data->current_array );
    result = TRUE;
  }

  return result;
}

int jxtl_parser_parse_buffer( parser_t *parser, const char *buffer,
                              jxtl_template_t **template_ptr )
{
  jxtl_callback_t *jxtl_callbacks = parser_get_user_data( parser );
  jxtl_data_t *jxtl_data = (jxtl_data_t *) jxtl_callbacks->user_data;
  int result = FALSE;

  *template_ptr = NULL;
  jxtl_data_reset( jxtl_data );

  if ( parser_parse_buffer( parser, buffer ) ) {
    *template_ptr = jxtl_template_create( parser->mp,
                                          jxtl_data->current_array );
    result = TRUE;
  }

  return result;
}
