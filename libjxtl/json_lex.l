%{
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_tables.h>

#include "apr_macros.h"

#include "json_node.h"
#include "json.h"

#include "json_parse.h"
#include "lex_extra.h"
#include "utf.h"

#define YY_DECL int json_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param, yyscan_t yyscanner )

#define LEX_EXTRA ((lex_extra_t *) yyextra)
#define LEX_EXTRA_MP LEX_EXTRA->mp
#define LEX_EXTRA_STR_ARRAY LEX_EXTRA->str_array
#define LEX_EXTRA_STATUS LEX_EXTRA->status
#define LEX_EXTRA_IN_FILE LEX_EXTRA->in_file
#define LEX_EXTRA_BYTES LEX_EXTRA->bytes

#define json_lex_error( ... ) \
  json_error( yylloc, yyscanner, NULL, __VA_ARGS__ )

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
%}

%option prefix="json_"
%option header-file="json_lex.h"
%option outfile="json_lex.c"
%option noyywrap
%option yylineno
%option reentrant
%option bison-bridge
%option bison-locations
%option debug

%x unmatched squote_str dquote_str
   
unicode_escape ("\\u"[0-9a-fA-F]{4}){1,2}
integer "-"?([0-9]|[1-9][0-9]+)
frac "."[0-9]+
exp ("e"|"E")("+"|"-")?[0-9]+

%%

<INITIAL>{
  "'" { APR_ARRAY_CLEAR( LEX_EXTRA_STR_ARRAY ); BEGIN( squote_str ); }
  "\"" { APR_ARRAY_CLEAR( LEX_EXTRA_STR_ARRAY ); BEGIN( dquote_str ); }
  "{" { return '{'; }
  "}" { return '}'; }
  "[" { return '['; }
  "]" { return ']'; }
  "," { return ','; }
  ":" { return ':'; }
  "false" { return T_FALSE; }
  "true" { return T_TRUE; }
  "null" { return T_NULL; }
  {integer} {
    yylval->integer = strtol( yytext, NULL, 10 );
    return T_INTEGER;
  }
  {integer}{frac} {
    yylval->number = strtod( yytext, NULL );
    return T_NUMBER;
  }
  {integer}{exp} {
    yylval->number = strtod( yytext, NULL );
    return T_NUMBER;
  }
  {integer}{frac}{exp} {
    yylval->number = strtod( yytext, NULL );
    return T_NUMBER;
  }
  [ \t\r]+
  "\n"
  . {
    yyless( 0 );
    yycolumn--;
    BEGIN( unmatched );
  }
}

<unmatched>{
  [^\'\"{}\[\],:0-9\t\n\r ]+ {
    json_lex_error( "unexpected \"%.*s\"", yyleng, yytext );
    BEGIN( INITIAL );
  }
}

<dquote_str>{
  "\"" {
    BEGIN( INITIAL );
    yylval->string = apr_palloc( LEX_EXTRA_MP,
				 LEX_EXTRA_STR_ARRAY->nelts + 1 );
    utf8_strcpyn( yylval->string, (unsigned char *) LEX_EXTRA_STR_ARRAY->elts,
                  LEX_EXTRA_STR_ARRAY->nelts );
    return T_STRING;
  }
  "'" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\''; }
  "\\\"" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '"'; }
}
<squote_str>{
  "'" {
    BEGIN( INITIAL );
    yylval->string = apr_palloc( LEX_EXTRA_MP,
				 LEX_EXTRA_STR_ARRAY->nelts + 1 );
    utf8_strcpyn( yylval->string, (unsigned char *) LEX_EXTRA_STR_ARRAY->elts,
                  LEX_EXTRA_STR_ARRAY->nelts );
    return T_STRING;
  }
  "\"" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '"'; }
  "\\\'" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\''; }
}

<squote_str,dquote_str>{
  "\\\\" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\\'; }
  "\\/" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '/'; }
  "\\b" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\b'; }
  "\\f" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\f'; }
  "\\n" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\n'; }
  "\\r" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\r'; }
  "\\t" { APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = '\t'; }
  {unicode_escape} {
    int value;
    int value2;
    char tmp_str[4];
    unsigned char utf8_str[5];
    int i;

    memcpy( tmp_str, yytext + 2, 4 );
    value = strtol( tmp_str, NULL, 16 );

    if ( yyleng > 6 ) {
      memcpy( tmp_str, yytext + 8, 4 );
      value2 = strtol( tmp_str, NULL, 16 );
      if ( value >= 0xD800 && value <= 0xDBFF ) {
	/* Valid first surrogate */
	if ( value2 >= 0xDC00 && value2 <= 0xDFFF ) {
	  /* Valid second surrogate */
	  value = ( ( value - 0xD800 ) << 10 ) + ( value2 - 0xDC00 ) + 0x10000;
	  utf8_encode( value, utf8_str );
	  for ( i = 0; utf8_str[i]; i++ ) {
	    APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = utf8_str[i];
	  }
	}
	else {
	  json_lex_error( "invalid unicode \\u%.4x\\u%.4x", value, value2 );
	}
      }
      else if ( value <= 0x7F ) {
	/* Not a surrogate pair */
	yyless( 6 );
	yycolumn = yycolumn - 6;
	
	utf8_encode( value, utf8_str );
	for ( i = 0; utf8_str[i]; i++ ) {
	  APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = utf8_str[i];
	}
      }
      else {
	json_lex_error( "invalid unicode \\u%.4x\\u%.4x", value, value2 );
      }
    }
    else if ( value >= 0xD800 && value <= 0xDFFF ) {
      /* These are invalid UTF-8 characters, only can be used as surrogate
	 pairs */
      json_lex_error( "invalid unicode \\u%.4x", value );
    }
    else {
      utf8_encode( value, utf8_str );
      for ( i = 0; utf8_str[i]; i++ ) {
	APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = utf8_str[i];
      }
    }
  }
  [\x00-\x09\x0b-\x1f] {
    /*
     * Not legal as per the JSON specification, save it off and issue an
     * error.
     */
    APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = yytext[0];
    json_lex_error( "illegal control character 0x%x", yytext[0] );
  }
  [^\"\'\\\x00-\x1f]+ {
    int i;
    for ( i = 0; i < yyleng; i++ ) {
      APR_ARRAY_PUSH( LEX_EXTRA_STR_ARRAY, char ) = yytext[i];
    }
  }
  . {
    json_lex_error( "start of illegal backslash" );
  }
  "\n" {
    json_lex_error( "unterminated string constant" );
  }
}

%%
