/*
 * json_parse.y
 *
 * Description
 *   Bison source file for generating the JSON grammar.
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

/*
 * Define YY_DECL before including json_lex.h so that it knows we are doing a
 * custom declaration of json_lex.
 */
#define YY_DECL

#include "json_parse.h"
#include "json_lex.h"

#include "json.h"
#include "json_writer.h"
#include "parser.h"

#define callbacks ((json_callback_t *) callbacks_ptr)

int json_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param,
              yyscan_t yyscanner );
void json_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                 void *callbacks_ptr, const char *error_string, ... );
%}

%name-prefix="json_"
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
  int integer;
  char *string;
  double number;
}

%token T_TRUE "true" T_FALSE "false" T_NULL "null"
%token <integer> T_INTEGER "integer"
%token <number> T_NUMBER "number"
%token <string> T_STRING "string"

%%

object_or_array
  : object
  | array
;

object
  : '{' { callbacks->object_start_handler( callbacks->user_data ); }
    '}' { callbacks->object_end_handler( callbacks->user_data ); }
  | '{' { callbacks->object_start_handler( callbacks->user_data ); }
    members
    '}' { callbacks->object_end_handler( callbacks->user_data ); }
  | '{' { callbacks->object_start_handler( callbacks->user_data ); }
    error
    '}' { callbacks->object_end_handler( callbacks->user_data ); }
;

members
  : pair
  | members ',' pair
;

pair
  : T_STRING { callbacks->property_start_handler( callbacks->user_data,
                                                  $<string>1 ); }
    ':' value { callbacks->property_end_handler( callbacks->user_data ); }
;

array
  : '[' { callbacks->array_start_handler( callbacks->user_data ); }
    ']' { callbacks->array_end_handler( callbacks->user_data ); }
  | '[' { callbacks->array_start_handler( callbacks->user_data ); }
    elements
    ']' { callbacks->array_end_handler( callbacks->user_data ); }
  | '[' { callbacks->array_start_handler( callbacks->user_data ); }
    error
    ']' { callbacks->array_end_handler( callbacks->user_data ); }
;

elements
  : value
  | elements ',' value
;

value
  : T_STRING { callbacks->string_handler( callbacks->user_data, $<string>1 ); }
  | T_INTEGER { callbacks->integer_handler( callbacks->user_data,
                                            $<integer>1 ); }
  | T_NUMBER { callbacks->number_handler( callbacks->user_data, $<number>1 ); }
  | object
  | array
  | T_TRUE { callbacks->boolean_handler( callbacks->user_data, 1 ); }
  | T_FALSE { callbacks->boolean_handler( callbacks->user_data, 0 ); }
  | T_NULL { callbacks->null_handler( callbacks->user_data ); }
;

%%

void json_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                 void *callbacks_ptr, const char *error_string, ... )
{
  va_list args;

  fprintf( stderr, "%s:%d,%d-%d: ", parser->get_filename( parser ),
           yylloc->first_line, yylloc->first_column, yylloc->last_column );
  va_start( args, error_string );
  vfprintf( stderr, error_string, args );
  va_end( args );
  fprintf( stderr, "\n" );
}

parser_t *json_parser_create( apr_pool_t *mp )
{
  return parser_create( mp,
                        json_lex_init,
                        json_set_extra,
                        json_lex_destroy,
                        json__scan_buffer,
                        json__delete_buffer,
                        json_parse );
}

int json_parser_parse_file( parser_t *parser, const void *file,
                            json_callback_t *json_callbacks )
{
  parser_set_user_data( parser, json_callbacks );
  return parser_parse_file( parser, (apr_file_t *) file );
}

int json_parser_parse_buffer( parser_t *parser, const void *buffer,
                              json_callback_t *json_callbacks )
{
  parser_set_user_data( parser, json_callbacks );
  return parser_parse_buffer( parser, (const char *) buffer );
}
