%{
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "parser.h"
#include "jxtl_parse.h"
#include "parser_lex_macros.h"
#include "utf.h"

#define YY_DECL int jxtl_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param, yyscan_t yyscanner )

#define jxtl_error( ... ) \
  jxtl_error( yylloc, yyscanner, NULL, __VA_ARGS__ )

#define STRING_COPY {                                           \
    int i;                                                      \
    for ( i = 0; i < yyleng; i++ ) {                            \
      APR_ARRAY_PUSH( PARSER_STR_ARRAY, char ) = yytext[i];     \
    }                                                           \
  }
%}

%option prefix="jxtl_"
%option header-file="jxtl_lex.h"
%option outfile="jxtl_lex.c"
%option noyywrap
%option yylineno
%option reentrant
%option bison-bridge
%option bison-locations
%option case-insensitive

%x directive options str unmatched path_expr comment

%%

<INITIAL>{
  "{{" {
    /*
     * Wait as long as possible to return text so that it can be collected
     * all together.  If there is some saved texted in the str array, then
     * return that now and put the the directive start text back so that it
     * can be returned next time after the array is cleared.
     */
    if ( PARSER_STR_ARRAY->nelts > 0 )  {
      yylval->string = apr_pstrndup( PARSER_MP, PARSER_STR_ARRAY->elts,
				     PARSER_STR_ARRAY->nelts );
      APR_ARRAY_CLEAR( PARSER_STR_ARRAY );
      yyless( 0 );
      return T_TEXT;
    }
    else {
      BEGIN( directive );
      return T_DIRECTIVE_START;
    }
  }
  "{{!" {
    BEGIN( comment );
  }
  "{"|"\n" { STRING_COPY; }
  [^{\n]+ { STRING_COPY; }
}

<directive>{
  "#section" { return T_SECTION; }
  "#end" { return T_END; }
  "#if" { return T_IF; }
  "#else" { return T_ELSE; }
  "#elseif" { return T_ELSEIF; }
  [a-zA-Z0-9_/.*\[\]!]+ {
    yylval->string = apr_pstrndup( PARSER_MP, yytext, yyleng );
    return T_PATH_EXPR;
  }
  ";" { BEGIN( options ); }
  "}}" { BEGIN( INITIAL ); return T_DIRECTIVE_END; }
  [ \t\r]+
  "\n"
  . { jxtl_error( "illegal character '%c' found in path expression",
		  yytext[0] ); }
}

<options>{
  "separator" { return T_SEPARATOR; }
  "=" { return '='; }
  "," { return ','; }
  "}}" { BEGIN( INITIAL ); return T_DIRECTIVE_END; }
  "\"" {
    APR_ARRAY_CLEAR( PARSER_STR_ARRAY );
    BEGIN( str );
  }
  [ \t\r]+
  "\n"
  . {
    yyless( 0 );
    BEGIN( unmatched );
  }
}

<str>{
  "\"" {
    yylval->string = apr_pstrndup( PARSER_MP,
                                   PARSER_STR_ARRAY->elts,
                                   PARSER_STR_ARRAY->nelts );
    APR_ARRAY_CLEAR( PARSER_STR_ARRAY );
    BEGIN( options );
    return T_STRING;
  }
  "\\b" { APR_ARRAY_PUSH( PARSER_STR_ARRAY, char ) = '\b'; }
  "\\f" { APR_ARRAY_PUSH( PARSER_STR_ARRAY, char ) = '\f'; }
  "\\n" { APR_ARRAY_PUSH( PARSER_STR_ARRAY, char ) = '\n'; }
  "\\r" { APR_ARRAY_PUSH( PARSER_STR_ARRAY, char ) = '\r'; }
  "\\t" { APR_ARRAY_PUSH( PARSER_STR_ARRAY, char ) = '\t'; }
  "\\\"" { APR_ARRAY_PUSH( PARSER_STR_ARRAY, char ) = '"'; }
  [^\"\\\n]+ {
    STRING_COPY;
  }
  "\n" {
    jxtl_error( "unterminated string constant" );
    BEGIN( INITIAL );
  }
}

<unmatched>{
  [^ \t\r\n,}]+ {
    jxtl_error( "bad option '%s'", yytext );
    BEGIN( options );
  }
}

<INITIAL><<EOF>> {
  if ( PARSER_STR_ARRAY->nelts > 0 )  {
    yylval->string = apr_pstrndup( PARSER_MP,
				   PARSER_STR_ARRAY->elts,
				   PARSER_STR_ARRAY->nelts );
    APR_ARRAY_CLEAR( PARSER_STR_ARRAY );
    return T_TEXT;
  }
  yyterminate();
}

<comment>{
  "-}}"[\n]? { BEGIN( INITIAL ); }
  "!}}" { BEGIN( INITIAL ); }
  [^!\-\n]+
  "!"|"-"
  "\n"
}

%%
