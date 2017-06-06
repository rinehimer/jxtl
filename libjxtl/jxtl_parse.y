/*
 * jxtl_parse.y
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
#include "parser.h"
#include "jxtl_parse.h"
#include "jxtl_lex.h"
#include "jxtl.h"

#define callbacks ((jxtl_callback_t *) callbacks_ptr)

int jxtl_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param,
              yyscan_t yyscanner );
void jxtl_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                 void *callbacks_ptr, const char *error_string, ... );
%}

%name-prefix "jxtl_"
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
  char *string;
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
      if ( callbacks->text_handler ) {
        callbacks->text_handler( callbacks->user_data, $<string>2 );
      }
    }
  | text value_directive
  | text section_directive
  | text if_directive
;

value_directive
  : T_DIRECTIVE_START T_PATH_EXPR
    {
      if ( callbacks->value_handler &&
           !callbacks->value_handler( callbacks->user_data, $<string>2 ) ) {
        jxtl_error( &@2, scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    options T_DIRECTIVE_END
;

section_directive
  : T_DIRECTIVE_START T_SECTION T_PATH_EXPR
    {
      if ( callbacks->section_start_handler &&
          !callbacks->section_start_handler( callbacks->user_data,
                                              $<string>3 ) ) {
        jxtl_error( &@2, scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    options T_DIRECTIVE_END section_content
    T_DIRECTIVE_START T_END T_DIRECTIVE_END
    { 
      if ( callbacks->section_end_handler ) {
        callbacks->section_end_handler( callbacks->user_data );
      }
    }
;

if_directive
  : T_DIRECTIVE_START T_IF T_PATH_EXPR T_DIRECTIVE_END
    {
      if ( callbacks->if_start_handler &&
           !callbacks->if_start_handler( callbacks->user_data, $<string>3 ) ) {
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
      if ( callbacks->elseif_handler &&
           !callbacks->elseif_handler( callbacks->user_data, $<string>3 ) ) {
        jxtl_error( &@3, scanner, parser, callbacks_ptr,
                    callbacks->get_error_func( callbacks->user_data ) );
      }
    }
    section_content rest_of_if
  | T_DIRECTIVE_START T_ELSE T_DIRECTIVE_END
    {
      if ( callbacks->else_handler ) {
        callbacks->else_handler( callbacks->user_data );
      }
    }
    section_content endif
  | endif
  ;

endif
  : T_DIRECTIVE_START T_END T_DIRECTIVE_END
    {
      if ( callbacks->if_end_handler ) {
        callbacks->if_end_handler( callbacks->user_data );
      }
    }

section_content
  : /* empty */
  | section_content T_TEXT
    {
      if ( callbacks->text_handler ) {
        callbacks->text_handler( callbacks->user_data, $<string>2 );
      }
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
      if ( callbacks->separator_start_handler ) {
        callbacks->separator_start_handler( callbacks->user_data );
      }
      if ( callbacks->text_handler ) {
        callbacks->text_handler( callbacks->user_data, $<string>3 );
      }
      if ( callbacks->separator_end_handler ) {
        callbacks->separator_end_handler( callbacks->user_data );
      }
    }
  | T_FORMAT '=' T_STRING
    {
      if ( callbacks->format_handler ) {
        callbacks->format_handler( callbacks->user_data, $<string>3 );
      }
    }
;

%%

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

parser_t *jxtl_parser_create( apr_pool_t *mp )
{
  return parser_create( mp,
			jxtl_lex_init,
			jxtl_set_extra,
			jxtl_lex_destroy,
			jxtl__scan_buffer,
			jxtl__delete_buffer,
			jxtl_parse );
}

int jxtl_parser_parse_file( parser_t *parser, apr_file_t *file,
			    jxtl_callback_t *jxtl_callbacks )
{
  parser_set_user_data( parser, jxtl_callbacks );
  return parser_parse_file( parser, file );
}

int jxtl_parser_parse_buffer( parser_t *parser, const char *buffer,
			      jxtl_callback_t *jxtl_callbacks )
{
  parser_set_user_data( parser, jxtl_callbacks );
  return parser_parse_buffer( parser, buffer );
}
