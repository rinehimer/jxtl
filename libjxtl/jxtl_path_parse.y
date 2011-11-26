/*
 * $Id$
 *
 * Description
 *   Bison source file for generating the jxtl_path grammar.
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
#include <apr_strings.h>
#include <stdarg.h>

/*
 * Define YY_DECL before including jxtl_path_lex.h so that it knows we are
 * doing a custom declaration of jxtl_path_lex.
 */
#define YY_DECL

#include "apr_macros.h"
#include "parser.h"
#include "jxtl_path_parse.h"
#include "jxtl_path_lex.h"
#include "jxtl_path.h"

#define callbacks ((jxtl_path_callback_t *) callbacks_ptr)

int jxtl_path_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param,
                   yyscan_t yyscanner );
void jxtl_path_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                      void *callbacks_ptr, const char *error_string, ... );
%}

%name-prefix="jxtl_path_"
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

%token T_IDENTIFIER "identifier" T_PARENT ".."

%left '/'
%nonassoc '!'

%%

path_expr
  : '!' pattern { callbacks->negate_handler( callbacks->user_data ); }
  | pattern
;

pattern
  : '/' { callbacks->root_object_handler( callbacks->user_data ); } path_pattern
  | path_pattern
;

path_pattern
  : T_IDENTIFIER { callbacks->identifier_handler( callbacks->user_data,
                                                  $<string>1 ); } predicate
  | '.' { callbacks->current_object_handler( callbacks->user_data ); }
  | T_PARENT { callbacks->parent_object_handler( callbacks->user_data ); }
  | '*' { callbacks->any_object_handler( callbacks->user_data ); } predicate
  | path_pattern '/' path_pattern
;

predicate
  : /* empty */
  | '[' { callbacks->predicate_start_handler( callbacks->user_data ); }
     path_expr ']'
     { callbacks->predicate_end_handler( callbacks->user_data ); }
;

%%

/**
 * Parser error callback.  Just store the error so that the caller can retrieve
 * it later and show it in a context that makes sense.
 */
void jxtl_path_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                      void *callbacks_ptr, const char *error_string, ... )
{
  va_list args;
  va_start( args, error_string );
  str_buf_vprintf( parser->err_buf, error_string, args );
  va_end( args );
}

/**
 * Create a path parser.
 */
parser_t *jxtl_path_parser_create( apr_pool_t *mp )
{
  return parser_create( mp,
                        jxtl_path_lex_init,
                        jxtl_path_set_extra,
                        jxtl_path_lex_destroy,
                        jxtl_path__scan_buffer,
                        jxtl_path__delete_buffer,
                        jxtl_path_parse );
}

int jxtl_path_parser_parse_buffer( parser_t *parser,
                                   const char *path,
                                   jxtl_path_callback_t *path_callbacks )
{
  parser_set_user_data( parser, path_callbacks );
  return parser_parse_buffer( parser, path );
}
