/*
 * Define some utility macros for accessing pieces of the parser for using in
 * a flex source file.  Should only be included in a flex source file as these
 * macros depend on the macro yyextra being defined.
 */

#ifndef PARSER_LEX_MACROS_H
#define PARSER_LEX_MACROS_H

#include "parser.h"

#define PARSER ((parser_t *) yyextra)
#define PARSER_MP PARSER->mp
#define PARSER_STR_ARRAY PARSER->str_array
#define PARSER_STATUS PARSER->status
#define PARSER_IN_FILE PARSER->in_file
#define PARSER_BYTES PARSER->bytes

#define YY_INPUT( buf, result, max_size ) {                             \
    PARSER_BYTES = max_size;                                            \
    PARSER_STATUS = apr_file_read( PARSER_IN_FILE, buf, &PARSER_BYTES ); \
    result = ( PARSER_STATUS == APR_SUCCESS ) ? PARSER_BYTES : YY_NULL; \
 }

#define YY_USER_ACTION {						\
    yylloc->first_line = yylineno;					\
    yylloc->last_line = yylineno;					\
    yylloc->first_column = yycolumn + 1;				\
    yylloc->last_column = yycolumn + yyleng;				\
    if ( yyleng == 1 && yytext[0] == '\n' ) {				\
      yylloc->first_column = 0;						\
      yylloc->last_column = 0;						\
    }									\
    else {								\
      yycolumn += yyleng;						\
    }									\
  }

#endif
