/*
 * $Id$
 *
 * Description
 *   The parser implementation.
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

#include <apr_pools.h>
#include <apr_strings.h>

#include "apr_macros.h"
#include "str_buf.h"
#include "parser.h"

static const char *get_filename( parser_t *parser )
{
  const char *filename = NULL;
  if ( parser->in_file ) {
    apr_file_name_get( &filename, parser->in_file );
  }

  if ( !filename ) {
    filename = "(stdin)";
  }

  return filename;
}

parser_t *parser_create( apr_pool_t *mp,
                         flex_init_func flex_init,
                         flex_set_extra_func flex_set_extra,
                         flex_destroy_func flex_destroy,
                         flex_scan_buffer_func flex_scan,
                         flex_delete_buffer_func flex_delete,
                         bison_parse_func bison_parse )
{
  parser_t *parser = apr_palloc( mp, sizeof(parser_t) );
  parser->mp = mp;
  parser->user_data = NULL;
  parser->get_filename = get_filename;
  parser->flex_init = flex_init;
  parser->flex_set_extra = flex_set_extra;
  parser->flex_destroy = flex_destroy;
  parser->flex_scan = flex_scan;
  parser->flex_delete = flex_delete;
  parser->bison_parse = bison_parse;

  parser->str_buf = str_buf_create( mp, 8192 );
  parser->err_buf = str_buf_create( mp, 1024 );
  parser->flex_init( &parser->scanner );
  parser->flex_set_extra( parser, parser->scanner );
  apr_pool_cleanup_register( mp, parser->scanner, flex_destroy,
                             apr_pool_cleanup_null );
  return parser;
}

/**
 * Function to reset internal variables.
 */
static void reset_parser( parser_t *parser )
{
  parser->in_file = NULL;
  parser->line_num = 1;
  STR_BUF_CLEAR( parser->err_buf );
  STR_BUF_CLEAR( parser->str_buf );
}

void parser_set_user_data( parser_t *parser, void *user_data )
{
  parser->user_data = user_data;
}

void *parser_get_user_data( parser_t *parser )
{
  return parser->user_data;
}

char *parser_get_error( parser_t *parser )
{
  /* Make sure we NULL terminate it before returning. */
  str_buf_putc( parser->err_buf, '\0' );
  return parser->err_buf->data;
}

int parser_parse_file( parser_t *parser, const char *file )
{
  apr_status_t status;
  int is_stdin;
  char error_str[1024];
  int result = FALSE;

  reset_parser( parser );

  is_stdin = ( file && apr_strnatcasecmp( file, "-" ) == 0 );

  if ( !is_stdin ) {
    status = apr_file_open( &parser->in_file, file, APR_READ | APR_BUFFERED, 0,
                            parser->mp );
  }
  else if ( file && is_stdin ) {
    status = apr_file_open_stdin( &parser->in_file, parser->mp );
  }

  if ( status != APR_SUCCESS ) {
    apr_strerror( status, error_str, sizeof(error_str) );
    str_buf_append( parser->err_buf, error_str );
  }
  else {
    result = parser->bison_parse( parser->scanner, parser,
                                  parser->user_data ) == 0;
  }

  return result;
}

int parser_parse_buffer( parser_t *parser, const char *buffer )
{
  char *flex_buffer;
  int flex_buffer_len = strlen( buffer ) + 2;
  YY_BUFFER_STATE buffer_state;
  int result = FALSE;

  reset_parser( parser );

  flex_buffer = apr_palloc( parser->mp, flex_buffer_len );
  apr_cpystrn( flex_buffer, buffer, flex_buffer_len - 1 );
  flex_buffer[flex_buffer_len - 1] = '\0';

  buffer_state = parser->flex_scan( flex_buffer, flex_buffer_len,
                                    parser->scanner );

  result = parser->bison_parse( parser->scanner, parser,
                                parser->user_data ) == 0;

  parser->flex_delete( buffer_state, parser->scanner );

  return result;
}
