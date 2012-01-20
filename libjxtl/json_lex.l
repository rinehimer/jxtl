/*
 * json_lex.l
 *
 * Description
 *   The lexer for JSON.
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
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_tables.h>

#include "apr_macros.h"

#include "json_parse.h"

#include "parser.h"
#include "json.h"

#include "parser_lex_macros.h"
#include "utf.h"

#define YY_DECL int json_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param, yyscan_t yyscanner )

#define json_lex_error( ... ) \
  json_error( yylloc, yyscanner, PARSER, NULL, __VA_ARGS__ )

void json_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                 void *callbacks_ptr, const char *error_string, ... );
%}

%option prefix="json_"
%option header-file="json_lex.h"
%option outfile="json_lex.c"
%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations
%option nounput

%x unmatched squote_str dquote_str

unicode_escape ("\\u"[0-9a-fA-F]{4}){1,2}
integer "-"?([0-9]|[1-9][0-9]+)
frac "."[0-9]+
exp ("e"|"E")("+"|"-")?[0-9]+

%%

<INITIAL>{
  "'" { STR_BUF_CLEAR( PARSER_STR_BUF ); BEGIN( squote_str ); }
  "\"" { STR_BUF_CLEAR( PARSER_STR_BUF ); BEGIN( dquote_str ); }
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
    PARSER_LESS( 0 );
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
    yylloc->first_column = yylloc->last_column - PARSER_STR_BUF->data_len - 1;
    yylval->string = apr_palloc( PARSER_MP, PARSER_STR_BUF->data_len + 1 );
    utf8_strcpyn( yylval->string, PARSER_STR_BUF->data,
                  PARSER_STR_BUF->data_len );
    return T_STRING;
  }
  "'" { str_buf_putc( PARSER_STR_BUF, '\'' ); }
  "\\\"" { str_buf_putc( PARSER_STR_BUF, '"' ); }
}
<squote_str>{
  "'" {
    BEGIN( INITIAL );
    yylloc->first_column = yylloc->last_column - PARSER_STR_BUF->data_len - 1;
    yylval->string = apr_palloc( PARSER_MP, PARSER_STR_BUF->data_len + 1 );
    utf8_strcpyn( yylval->string, PARSER_STR_BUF->data,
                  PARSER_STR_BUF->data_len );
    return T_STRING;
  }
  "\"" { str_buf_putc( PARSER_STR_BUF, '"' ); }
  "\\\'" { str_buf_putc( PARSER_STR_BUF, '\'' ); }
}

<squote_str,dquote_str>{
  "\\\\" { str_buf_putc( PARSER_STR_BUF, '\\' ); }
  "\\/" { str_buf_putc( PARSER_STR_BUF, '/' ); }
  "\\b" { str_buf_putc( PARSER_STR_BUF, '\b' ); }
  "\\f" { str_buf_putc( PARSER_STR_BUF, '\f' ); }
  "\\n" { str_buf_putc( PARSER_STR_BUF, '\n' ); }
  "\\r" { str_buf_putc( PARSER_STR_BUF, '\r' ); }
  "\\t" { str_buf_putc( PARSER_STR_BUF, '\t' ); }
  {unicode_escape} {
    int value;
    int value2;
    char tmp_str[4];
    unsigned char utf8_str[5];

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
          str_buf_append( PARSER_STR_BUF, utf8_str );
        }
        else {
          json_lex_error( "invalid unicode \\u%.4x\\u%.4x", value, value2 );
        }
      }
      else if ( value <= 0x7F ) {
        /* Not a surrogate pair */
        PARSER_LESS( 6 );

        utf8_encode( value, utf8_str );
        str_buf_append( PARSER_STR_BUF, utf8_str );
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
      str_buf_append( PARSER_STR_BUF, utf8_str );
    }
  }
  [\x00-\x09\x0b-\x1f] {
    /*
     * Not legal as per the JSON specification, save it off and issue an
     * error.
     */
    str_buf_putc( PARSER_STR_BUF, yytext[0] );
    json_lex_error( "illegal control character 0x%x", yytext[0] );
  }
  [^\"\'\\\x00-\x1f]+ {
    str_buf_write( PARSER_STR_BUF, yytext, yyleng );
  }
  . {
    json_lex_error( "start of illegal backslash" );
  }
  "\n" {
    /* Unterminated string constant, still return the string for the parser. */
    BEGIN( INITIAL );
    json_lex_error( "unterminated string constant" );
    yylval->string = apr_palloc( PARSER_MP,
                                 PARSER_STR_BUF->data_len + 1 );
    utf8_strcpyn( yylval->string, PARSER_STR_BUF->data,
                  PARSER_STR_BUF->data_len );
    return T_STRING;
  }
}

%%
