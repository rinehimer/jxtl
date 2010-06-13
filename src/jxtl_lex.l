%{
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"

#include "json_node.h"
#include "json.h"

#include "jxtl_parse.h"
#include "lex_extra.h"
#include "utf.h"

#define YY_DECL int jxtl_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param, yyscan_t yyscanner )

#define LEX_EXTRA ((lex_extra_t *) yyextra)
#define LEX_EXTRA_MP LEX_EXTRA->mp
#define LEX_EXTRA_STR_ARRAY LEX_EXTRA->str_array
#define LEX_EXTRA_STATUS LEX_EXTRA->status
#define LEX_EXTRA_IN_FILE LEX_EXTRA->in_file
#define LEX_EXTRA_BYTES LEX_EXTRA->bytes

#define jxtl_error( ... ) \
  jxtl_error( yylloc, yyscanner, NULL, __VA_ARGS__ )

#define YY_INPUT( buf, result, max_size ) {				\
    LEX_EXTRA_BYTES = max_size;                                         \
    LEX_EXTRA_STATUS = apr_file_read( LEX_EXTRA_IN_FILE, buf,           \
                                      &LEX_EXTRA_BYTES );               \
    result = ( LEX_EXTRA_STATUS == APR_SUCCESS ) ? LEX_EXTRA_BYTES : YY_NULL; \
 }

#define YY_USER_ACTION {						\
    yylloc->first_line = yylineno;                                      \
    yylloc->first_column = yycolumn;                                    \
    yycolumn = yylloc->first_column + yyleng;                           \
    yylloc->last_line = yylineno;                                       \
    yylloc->last_column = yycolumn - 1;                                 \
 }

#define STRING_COPY {                                           \
    int i;                                                      \
    for ( i = 0; i < yyleng; i++ ) {                            \
      APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = yytext[i];  \
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

%x directive options str unmatched expression comment

identifier ([a-zA-Z][a-zA-Z_.]+)|"@"

%%

<INITIAL>{
   "{{" {
     /*
      * Wait as long as possible to return text so that it can be collected
      * all together.  If there is some saved texted in the str array, then
      * return that now and put the the directive start text back so that it
      * can be returned next time after the array is cleared.
      */
     if ( LEX_EXTRA_STR_ARRAY->nelts > 0 )  {
       yylval->string = apr_pstrndup( LEX_EXTRA_MP,
                                      LEX_EXTRA_STR_ARRAY->elts,
                                      LEX_EXTRA_STR_ARRAY->nelts );
       APR_ARRAY_CLEAR( LEX_EXTRA_STR_ARRAY );
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
    "#if" { BEGIN( expression ); return T_IF; }
    ";" { BEGIN( options ); }
    {identifier} {
      yylval->string = apr_pstrdup( LEX_EXTRA_MP, yytext );
      return T_IDENTIFIER;
    }
    "}}" { BEGIN( INITIAL ); return T_DIRECTIVE_END; }
    [ \t\r]+
    "\n"
   . { jxtl_error( "illegal character '%c' found in identifier", yytext[0] ); }
}

<options>{
  "separator" { return T_SEPARATOR; }
  "test" { return T_TEST; }
  "=" { return '='; }
  "," { return ','; }
  "(" { BEGIN( expression ); return '('; }
  "}}" { BEGIN( INITIAL ); return T_DIRECTIVE_END; }
  "\"" {
    APR_ARRAY_CLEAR( LEX_EXTRA_STR_ARRAY );
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
    yylval->string = apr_pstrndup( LEX_EXTRA_MP,
                                   LEX_EXTRA_STR_ARRAY->elts,
                                   LEX_EXTRA_STR_ARRAY->nelts );
    APR_ARRAY_CLEAR( LEX_EXTRA_STR_ARRAY );
    BEGIN( options );
    return T_STRING;
  }
  "\\b" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\b'; }
  "\\f" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\f'; }
  "\\n" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\n'; }
  "\\r" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\r'; }
  "\\t" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\t'; }
  "\\\"" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '"'; }
  [^\"\\\n]+ {
    STRING_COPY;
  }
  "\n" {
    jxtl_error( "unterminated string constant" );
    BEGIN( INITIAL );
  }
}

<expression>{
  "!" { return '!'; }
  "(" { return '('; }
  ")" { BEGIN( options ); return ')'; }
  {identifier} {
    yylval->string = apr_pstrdup( LEX_EXTRA_MP, yytext );
    return T_IDENTIFIER;
  }
}

<unmatched>{
  [^ \t\r\n,}]+ {
    jxtl_error( "bad option '%s'", yytext );
    BEGIN( options );
  }
}

<INITIAL><<EOF>> {
  if ( LEX_EXTRA_STR_ARRAY->nelts > 0 )  {
    yylval->string = apr_pstrndup( LEX_EXTRA_MP,
				   LEX_EXTRA_STR_ARRAY->elts,
				   LEX_EXTRA_STR_ARRAY->nelts );
    APR_ARRAY_CLEAR( LEX_EXTRA_STR_ARRAY );
    return T_TEXT;
  }
  yyterminate();
}

<comment>{
  "!}}" { BEGIN( INITIAL ); }
  [^!\n]+
  "!"
  "\n"
}

%%
