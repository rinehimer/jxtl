/*
 * $Id$
 *
 * Description
 *   The lexer for path language used in the templates.
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
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"

#include "jxtl_path_parse.h"
#include "parser.h"
#include "parser_lex_macros.h"
#include "utf.h"

#define YY_DECL int jxtl_path_lex( YYSTYPE *yylval_param, YYLTYPE *yylloc_param, yyscan_t yyscanner )

#define jxtl_lex_error( ... ) \
  jxtl_path_error( yylloc, yyscanner, PARSER, NULL, __VA_ARGS__ )

void jxtl_path_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                      void *callbacks_ptr, const char *error_string, ... );
%}

%option prefix="jxtl_path_"
%option header-file="jxtl_path_lex.h"
%option outfile="jxtl_path_lex.c"
%option noyywrap
%option reentrant
%option bison-bridge
%option bison-locations
%option case-insensitive

identifier [a-zA-Z][a-zA-Z0-9_]*

%%

"/" { return '/'; }
"." { return '.'; }
".." { return T_PARENT; }
"*" { return '*'; }
"[" { return '['; }
"]" { return ']'; }
"!" { return '!'; }
{identifier} {
  yylval->string = apr_pstrdup( PARSER_MP, yytext );
  return T_IDENTIFIER;
}
[ \t\r]+
"\n"
. {
  jxtl_lex_error( "illegal character '%c' found in path expression",
                  yytext[0] );
}

%%
