%{
#include <stdarg.h>

/*
 * Define YY_DECL before including json_lex.h so that it knows we are doing a
 * custom declaration of json_lex.
 */
#define YY_DECL

#include "json_parse.h"
#include "json_lex.h"
#include "json_parser.h"

#define object_start_handler callbacks->object_start_handler
#define object_end_handler callbacks->object_end_handler
#define array_start_handler callbacks->array_start_handler
#define array_end_handler callbacks->array_end_handler
#define property_start_handler callbacks->property_start_handler
#define property_end_handler callbacks->property_end_handler
#define string_handler callbacks->string_handler
#define integer_handler callbacks->integer_handler
#define number_handler callbacks->number_handler
#define boolean_handler callbacks->boolean_handler
#define null_handler callbacks->null_handler
#define user_data callbacks->user_data

char *json_lex_get_filename( yyscan_t *yyscanner );
void json_error( YYLTYPE *yylloc, yyscan_t scanner, json_callback_t *callbacks,
		 const char *error_string, ... );
%}

%name-prefix="json_"
%defines
%verbose
%locations
%error-verbose

%pure-parser

%parse-param { yyscan_t scanner }
%parse-param { json_callback_t *callbacks }
%lex-param { yyscan_t scanner }

%union {
  int integer;
  unsigned char *string;
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
  : '{' { object_start_handler( user_data ); }
    '}' { object_end_handler( user_data ); }
  | '{' { object_start_handler( user_data ); }
    members
    '}' { object_end_handler( user_data ); }
  | '{' { object_start_handler( user_data ); }
    error
    '}' { object_end_handler( user_data ); }
;

members
  : pair
  | members ',' pair
;

pair
  : T_STRING { property_start_handler( user_data, $<string>1 ); }
    ':' value { property_end_handler( user_data ); }
;

array
  : '[' { array_start_handler( user_data ); }
    ']' { array_end_handler( user_data ); }
  | '[' { array_start_handler( user_data ); }
    elements
    ']' { array_end_handler( user_data ); }
  | '[' { array_start_handler( user_data ); }
    error
    ']' { array_end_handler( user_data ); }
;

elements
  : value
  | elements ',' value
;

value
  : T_STRING { string_handler( user_data, $<string>1 ); }
  | T_INTEGER { integer_handler( user_data, $<integer>1 ); }
  | T_NUMBER { number_handler( user_data, $<number>1 ); }
  | object
  | array
  | T_TRUE { boolean_handler( user_data, 1 ); }
  | T_FALSE { boolean_handler( user_data, 0 ); }
  | T_NULL { null_handler( user_data ); }
;

%%

void json_error( YYLTYPE *yylloc, yyscan_t scanner, json_callback_t *callbacks,
		 const char *error_string, ... )
{
  va_list args;
  fprintf( stderr, "%s: %d.%d-%d.%d ", json_lex_get_filename( scanner ),
	   yylloc->first_line, yylloc->first_column, yylloc->last_line,
	   yylloc->last_column );
  va_start( args, error_string);
  vfprintf( stderr, error_string, args );
  va_end( args );
  fprintf( stderr, "\n" );
}
