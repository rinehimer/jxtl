%{
#include <stdarg.h>

/*
 * Define YY_DECL before including json_path_lex.h so that it knows we are
 * doing a custom declaration of json_path_lex.
 */
#define YY_DECL

#define YYSTYPE char *

#include "json_path_parse.h"
#include "json_path_lex.h"
#include "json_path_parser.h"

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
  : T_IDENTIFIER
  | '$'
  | '@'
  | '(' path_expr ')'
  | '!' path_expr
  | path_expr '.' path_expr
;

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
