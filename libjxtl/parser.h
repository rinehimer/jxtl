/*
 * parser.h
 *
 * Description
 *   A generic parser API that is abstracts some of the details of using flex
 *   and bison.
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

#ifndef PARSER_H
#define PARSER_H

#include <apr_file_io.h>
#include "str_buf.h"

#ifndef YY_TYPEDEF_YY_SCANNER_T
#define YY_TYPEDEF_YY_SCANNER_T
typedef void* yyscan_t;
#endif

#ifndef YY_TYPEDEF_YY_BUFFER_STATE
#define YY_TYPEDEF_YY_BUFFER_STATE
typedef struct yy_buffer_state *YY_BUFFER_STATE;
#endif

#ifndef YY_TYPEDEF_YY_SIZE_T
#define YY_TYPEDEF_YY_SIZE_T
typedef size_t yy_size_t;
#endif

typedef struct parser_t parser_t;

typedef int ( *flex_init_func )( yyscan_t *yyscanner );
typedef void ( *flex_set_extra_func )( void *user_defined,
                                       yyscan_t yyscanner );
typedef int ( *flex_destroy_func )( void *yyscanner );
typedef YY_BUFFER_STATE ( *flex_scan_buffer_func )( char *base,
                                                    yy_size_t size,
                                                    yyscan_t yyscanner );
typedef void ( *flex_delete_buffer_func )( YY_BUFFER_STATE b,
                                           yyscan_t yyscanner );

typedef int ( *bison_parse_func )( yyscan_t yyscanner, parser_t *parser,
                                   void *user_data );

struct parser_t {
  /* Memory pool for objects allocated in this structure. */
  apr_pool_t *mp;
  /* Status variable used when reading from in_file. */
  apr_status_t status;
  /* Array for storing strings in the lexer. */
  str_buf_t *str_buf;
  /* An APR file pointer. */
  apr_file_t *in_file;
  /* Number of bytes read from the file. */
  apr_size_t bytes;
  /* A scanner object. */
  yyscan_t scanner;
  /* Current line number.  Only works if newlines are matched separately. */
  int line_num;
  /* If an error occurs during parsing and we want to save it. */
  str_buf_t *err_buf;
  /* User data. */
  void *user_data;
  const char * ( *get_filename )( struct parser_t * );
  /* Pointers to scanner and parser functions. */
  flex_init_func flex_init;
  flex_set_extra_func flex_set_extra;
  flex_destroy_func flex_destroy;
  flex_scan_buffer_func flex_scan;
  flex_delete_buffer_func flex_delete;
  bison_parse_func bison_parse;
};

/**
 * Create a new parser.
 * @param mp A pool to allocate the parser from.
 * @param flex_init A function to call to initialize the scanner object.
 * @param flex_set_extra A function to set the extra data of the scanner object.
 * @param flex_destroy A function to call to destroy the scanner object.
 * @param flex_scan A function to call for scanning a buffer.
 * @param flex_delete A function to call to delete a buffer created by scanning.
 * @param bison_parse The bison function to call to do the parse.
 * @return The newly allocated parser.
 */
parser_t *parser_create( apr_pool_t *mp,
                         flex_init_func flex_init,
                         flex_set_extra_func flex_set_extra,
                         flex_destroy_func flex_destroy,
                         flex_scan_buffer_func flex_scan,
                         flex_delete_buffer_func flex_delete,
                         bison_parse_func bison_parse );

/**
 * Parse a file.
 * @param parser A parser.
 * @param file The filename to parse.
 * @return TRUE or FALSE.
 */
int parser_parse_file( parser_t *parser, apr_file_t *file );

/**
 * Parse a buffer.
 * @param parser A parser
 * @param A null terminated buffer to parse.
 * @return TRUE or FALSE.
 */
int parser_parse_buffer( parser_t *parser, const char *buffer );

/**
 * Set the user data for a parser.  This will be passed to the bison_parse_func
 * as the third parameter.
 * @param parser A parser.
 * @param user_data Pointer to the user data.
 */
void parser_set_user_data( parser_t *parser, void *user_data );

/**
 * Return the stored user data for this parser.
 * @param parser A parser.
 * @return A pointer to the user data.
 */
void *parser_get_user_data( parser_t *parser );

/**
 * Return the error saved off.
 * @param parser A parser.
 */
char *parser_get_error( parser_t *parser );

#endif
