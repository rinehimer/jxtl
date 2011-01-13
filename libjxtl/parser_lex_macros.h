/*
 * $Id$
 *
 * Description
 *   Defines some utility macros for accessing pieces of the parser for using
 *   in a flex source file.  Should only be included in a flex source file as
 *   these macros depend on the macro yyextra being defined.  We could access
 *   the flex variables through function calls, but we really want this code
 *   as efficient as possible.
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

#ifndef PARSER_LEX_MACROS_H
#define PARSER_LEX_MACROS_H

#include "parser.h"

#define PARSER ((parser_t *) yyextra)
#define PARSER_MP PARSER->mp
#define PARSER_STR_BUF ((str_buf_t *)PARSER->str_buf)
#define PARSER_STATUS PARSER->status
#define PARSER_IN_FILE PARSER->in_file
#define PARSER_BYTES PARSER->bytes
#define PARSER_LINE_NUM PARSER->line_num

#define YY_INPUT( buf, result, max_size ) {                             \
    PARSER_BYTES = max_size;                                            \
    PARSER_STATUS = apr_file_read( PARSER_IN_FILE, buf, &PARSER_BYTES ); \
    result = ( PARSER_STATUS == APR_SUCCESS ) ? PARSER_BYTES : YY_NULL; \
 }

#define YY_USER_ACTION {                                                \
    yylloc->first_line = PARSER_LINE_NUM;                               \
    yylloc->last_line = PARSER_LINE_NUM;                                \
    yylloc->first_column = yycolumn + 1;                                \
    yylloc->last_column = yycolumn + yyleng;                            \
    if ( yytext[yyleng - 1] == '\n' ) {                                 \
      PARSER_LINE_NUM++;                                                \
      yylloc->first_column = 0;                                         \
      yylloc->last_column = 0;                                          \
      yycolumn = 0;                                                     \
    }                                                                   \
    else {                                                              \
      yycolumn += yyleng;                                               \
    }                                                                   \
  }

/**
 * Wrapper around yyless so that we can correctly adjust columns.  This macro
 * does not check for newlines, so you shouldn't put back newlines.
 */
#define PARSER_LESS( n ) do {                   \
    int less = n;                               \
    yycolumn -= yyleng - less;                  \
    yyless( less );                             \
  } while ( 0 );


#endif
