/*
 * jxtl_lex.l
 *
 * Description
 *   The lexer for template language.
 *
 * Copyright 2010-2015 Dan Rinehimer
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
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "parser.h"
#include "jxtl_parse.h"
#include "parser_lex_macros.h"
#include "utf.h"

#define YY_DECL int jxtl_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param, yyscan_t yyscanner )

#define jxtl_lex_error( ... ) \
  jxtl_error( yylloc, yyscanner, PARSER, NULL, __VA_ARGS__ )

void jxtl_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                 void *callbacks_ptr, const char *error_string, ... );

%}

%option prefix="jxtl_"
%option header-file="jxtl_lex.h"
%option outfile="jxtl_lex.c"
%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations
%option case-insensitive
%option nounput

path_expr_chars [a-zA-Z0-9_:.*\[\]!]

%x directive close_directive path_expr options str unmatched comment

%%

<INITIAL>{
  "{{" {
    /*
     * Wait as long as possible to return text so that it can be collected
     * all together.  If there is some saved texted in the str array, then
     * return that now and put the the directive start text back so that it
     * can be returned next time after the array is cleared.
     */
    if ( PARSER_STR_BUF->data_len > 0 )  {
      yylval->string = apr_pstrndup( PARSER_MP, PARSER_STR_BUF->data,
                                     PARSER_STR_BUF->data_len );
      STR_BUF_CLEAR( PARSER_STR_BUF );
      PARSER_LESS( 0 );
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
  "}}" { return T_DIRECTIVE_END; }
  "{"|"}"|"\n" { str_buf_putc( PARSER_STR_BUF, yytext[0] ); }
  [^{}\n]+ { str_buf_write( PARSER_STR_BUF, yytext, yyleng ); }
}

<directive>{
  "foreach" {
    STR_BUF_CLEAR( PARSER_STR_BUF );
    BEGIN( path_expr );
    return T_FOREACH;
  }
  "if" {
    STR_BUF_CLEAR( PARSER_STR_BUF );
    BEGIN( path_expr );
    return T_IF;
  }
  "else" { return T_ELSE; }
  "elseif" {
    STR_BUF_CLEAR( PARSER_STR_BUF );
    BEGIN( path_expr );
    return T_ELSEIF;
  }
  {path_expr_chars} {
    STR_BUF_CLEAR( PARSER_STR_BUF );
    str_buf_putc( PARSER_STR_BUF, yytext[0] );
    BEGIN( path_expr );
  }
  "/" { BEGIN( close_directive ); return '/'; }
  "}}" { BEGIN( INITIAL ); return T_DIRECTIVE_END; }
  [ \t\r]+
  "\n"|" "
  . { jxtl_lex_error( "illegal character '%c' found inside directive",
                      yytext[0] ); }
}

<close_directive>{
  "foreach" { BEGIN( directive ); return T_FOREACH; }
  "if" { BEGIN( directive ); return T_IF; }
  "else" { BEGIN( directive ); return T_ELSE; }
  "elseif" { BEGIN( directive ); return T_ELSEIF; }
}

<path_expr>{
  {path_expr_chars}+ {
    str_buf_write( PARSER_STR_BUF, yytext, yyleng );
  }
  ";" {
    yylval->string = apr_pstrndup( PARSER_MP, PARSER_STR_BUF->data,
                                   PARSER_STR_BUF->data_len );
    STR_BUF_CLEAR( PARSER_STR_BUF );
    BEGIN( options );
    return T_PATH_EXPR;
  }
  "}}" {
    yylval->string = apr_pstrndup( PARSER_MP, PARSER_STR_BUF->data,
                                   PARSER_STR_BUF->data_len );
    STR_BUF_CLEAR( PARSER_STR_BUF );
    BEGIN( INITIAL );
    PARSER_LESS( 0 );
    return T_PATH_EXPR;
  }
  "\n"|" "
  . { jxtl_lex_error( "illegal character '%c' found in path expression",
                      yytext[0] ); }
 }

<options>{
  "separator" { return T_SEPARATOR; }
  "format" { return T_FORMAT; }
  "=" { return '='; }
  "," { return ','; }
  "/" { return '/'; }
  "}}" { BEGIN( INITIAL ); return T_DIRECTIVE_END; }
  "\"" {
    STR_BUF_CLEAR( PARSER_STR_BUF );
    BEGIN( str );
  }
  [ \t\r]+
  "\n"
  . {
    PARSER_LESS( 0 );
    BEGIN( unmatched );
  }
}

<str>{
  "\"" {
    yylval->string = apr_pstrndup( PARSER_MP, PARSER_STR_BUF->data,
                                   PARSER_STR_BUF->data_len );
    STR_BUF_CLEAR( PARSER_STR_BUF );
    BEGIN( options );
    return T_STRING;
  }
  "\\b" { str_buf_putc( PARSER_STR_BUF, '\b' ); }
  "\\f" { str_buf_putc( PARSER_STR_BUF, '\f' ); }
  "\\n" { str_buf_putc( PARSER_STR_BUF, '\n' ); }
  "\\r" { str_buf_putc( PARSER_STR_BUF, '\r' ); }
  "\\t" { str_buf_putc( PARSER_STR_BUF, '\t' ); }
  "\\\"" { str_buf_putc( PARSER_STR_BUF, '"' ); }
  [^\"\\\n]+ {
    str_buf_write( PARSER_STR_BUF, yytext, yyleng );
  }
  "\n" {
    jxtl_lex_error( "unterminated string constant" );
    BEGIN( INITIAL );
  }
}

<unmatched>{
  [^ \t\r\n,}]+ {
    jxtl_lex_error( "bad option '%s'", yytext );
    BEGIN( options );
  }
}

<INITIAL><<EOF>> {
  if ( PARSER_STR_BUF->data_len > 0 )  {
    yylval->string = apr_pstrndup( PARSER_MP,
                                   PARSER_STR_BUF->data,
                                   PARSER_STR_BUF->data_len );
    STR_BUF_CLEAR( PARSER_STR_BUF );
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
