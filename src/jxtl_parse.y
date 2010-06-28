%{
#include <stdarg.h>

/*
 * Define YY_DECL before including jxtl_lex.h so that it knows we are doing a
 * custom declaration of jxtl_lex.
 */
#define YY_DECL

#include "jxtl_parse.h"
#include "jxtl_lex.h"
#include "jxtl.h"

#define text_handler callbacks->text_handler
#define section_start_handler callbacks->section_start_handler
#define section_end_handler callbacks->section_end_handler
#define separator_start_handler callbacks->separator_start_handler
#define separator_end_handler callbacks->separator_end_handler
#define if_start_handler callbacks->if_start_handler
#define elseif_handler callbacks->elseif_handler
#define else_handler callbacks->else_handler
#define if_end_handler callbacks->if_end_handler
#define value_handler callbacks->value_handler
#define user_data callbacks->user_data

void jxtl_error( YYLTYPE *yylloc, yyscan_t scanner, jxtl_callback_t *callbacks,
                 const char *error_string, ... );
%}

%name-prefix="jxtl_"
%defines
%verbose
%locations
%error-verbose
%pure-parser

%parse-param { yyscan_t scanner }
%parse-param { jxtl_callback_t *callbacks }
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
  | text T_TEXT { text_handler( user_data, $<string>2 ); }
  | text value_directive
  | text section_directive
  | text if_directive
;

value_directive
  : T_DIRECTIVE_START T_PATH_EXPR T_DIRECTIVE_END
    { value_handler( user_data, $<string>2 ); }
;

section_directive
  : T_DIRECTIVE_START T_SECTION T_PATH_EXPR
    { section_start_handler( user_data, $<string>3 ); }
    options
    T_DIRECTIVE_END
    section_content
    T_DIRECTIVE_START T_END T_DIRECTIVE_END
    { section_end_handler( user_data ); }
;

if_directive
  : T_DIRECTIVE_START T_IF T_PATH_EXPR T_DIRECTIVE_END
    {
      if_start_handler( user_data, $<string>3 );
    }
    section_content
    rest_of_if

;

rest_of_if
  : T_DIRECTIVE_START T_ELSEIF T_PATH_EXPR T_DIRECTIVE_END
    {
      elseif_handler( user_data, $<string>3 );
    }
    section_content rest_of_if
  | T_DIRECTIVE_START T_ELSE T_DIRECTIVE_END
    {
      else_handler( user_data );
    }
    section_content endif
  | endif
  ;

endif
  : T_DIRECTIVE_START T_END T_DIRECTIVE_END
    {
      if_end_handler( user_data );
    }

section_content
  : /* empty */
  | section_content T_TEXT { text_handler( user_data, $<string>2 ); }
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
      separator_start_handler( user_data );
      text_handler( user_data, $<string>3 );
      separator_end_handler( user_data );
    }
;

%%

void jxtl_error( YYLTYPE *yylloc, yyscan_t scanner, jxtl_callback_t *callbacks,
                 const char *error_string, ... )
{
  va_list args;
  fprintf( stderr, "%d: ", yylloc->first_line );
  va_start( args, error_string);
  vfprintf( stderr, error_string, args );
  va_end( args );
  fprintf( stderr, " near column %d\n", yylloc->first_column + 1 );
}
