%{
#include <stdarg.h>

/*
 * Define YY_DECL before including json_path_lex.h so that it knows we are
 * doing a custom declaration of json_path_lex.
 */
#define YY_DECL

#include "json_path_parse.h"
#include "json_path_lex.h"
#include "json_path.h"

#define identifier_handler callbacks->identifier_handler
#define root_object_handler callbacks->root_object_handler
#define parent_object_handler callbacks->parent_object_handler
#define current_object_handler callbacks->current_object_handler
#define all_children_handler callbacks->all_children_handler
#define test_start_handler callbacks->test_start_handler
#define test_end_handler callbacks->test_end_handler
#define negate_handler callbacks->negate_handler
#define user_data callbacks->user_data

void json_path_error( YYLTYPE *yylloc, yyscan_t scanner,
		      json_path_callback_t *callbacks,
		      const char *error_string, ... );
%}

%name-prefix="json_path_"
%defines
%verbose
%locations
%error-verbose

%pure-parser

%parse-param { yyscan_t scanner }
%parse-param { json_path_callback_t *callbacks }
%lex-param { yyscan_t scanner }

%union {
  int ival;
  unsigned char *string;
}

%token T_IDENTIFIER "identifier"

%left '.'
%nonassoc '!'

%%

path_expr
  : T_IDENTIFIER { identifier_handler( user_data, $<string>1 ); }
  | '$' { root_object_handler( user_data ); }
  | '@' { current_object_handler( user_data ); }
  | '^' { parent_object_handler( user_data ); }
  | '*' { all_children_handler( user_data ); }
  | '(' { test_start_handler( user_data ); } path_filter ')'
        { test_end_handler( user_data ); }
  | path_expr '.' path_expr
;

path_filter
  : path_expr
  | '!' path_expr { negate_handler( user_data ); }

%%

void json_path_error( YYLTYPE *yylloc, yyscan_t scanner,
		      json_path_callback_t *callbacks,
		      const char *error_string, ... )
{
  va_list args;
  fprintf( stderr, "%d: ", yylloc->first_line );
  va_start( args, error_string);
  vfprintf( stderr, error_string, args );
  va_end( args );
  fprintf( stderr, " near column %d\n", yylloc->first_column + 1 );
}
