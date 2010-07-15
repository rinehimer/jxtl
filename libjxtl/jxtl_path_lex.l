%{
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"

#include "jxtl_path_parse.h"
#include "lex_extra.h"
#include "utf.h"

#define YY_DECL int jxtl_path_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param, yyscan_t yyscanner )

#define LEX_EXTRA ((lex_extra_t *) yyextra)
#define LEX_EXTRA_MP LEX_EXTRA->mp
#define LEX_EXTRA_STR_ARRAY LEX_EXTRA->str_array
#define LEX_EXTRA_STATUS LEX_EXTRA->status
#define LEX_EXTRA_IN_FILE LEX_EXTRA->in_file
#define LEX_EXTRA_BYTES LEX_EXTRA->bytes

#define jxtl_path_error( ... ) \
  jxtl_path_error( yylloc, yyscanner, NULL, __VA_ARGS__ )

#define YY_USER_ACTION {						\
    yylloc->first_line = yylineno;                                      \
    yylloc->first_column = yycolumn;                                    \
    yycolumn = yylloc->first_column + yyleng;                           \
    yylloc->last_line = yylineno;                                       \
    yylloc->last_column = yycolumn - 1;                                 \
 }
%}

%option prefix="jxtl_path_"
%option header-file="jxtl_path_lex.h"
%option outfile="jxtl_path_lex.c"
%option noyywrap
%option yylineno
%option reentrant
%option bison-bridge
%option bison-locations
%option case-insensitive

identifier [a-zA-Z][a-zA-Z0-9_]*

%%

"$" { return '$'; }
"@" { return '@'; }
"^" { return '^'; }
"." { return '.'; }
"*" { return '*'; }
"(" { return '('; }
")" { return ')'; }
"!" { return '!'; }
{identifier} {
  yylval->string = apr_pstrdup( LEX_EXTRA_MP, yytext );
  return T_IDENTIFIER;
}
[ \t\r]+
"\n"
. {
  jxtl_path_error( "illegal character '%c' found in path expression",
		   yytext[0] );
}

%%
