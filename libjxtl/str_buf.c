/*
 * $Id$
 *
 * Description
 *   Implementation of a dynamic string buffer.  Unlike almost every other
 *   type, this actually uses malloc/realloc/free to for its buffer.  The
 *   reason is that using a memory pool isn't very practical if the buffer has
 *   to grow many times.  In that case, memory from the pool would just be
 *   wasted because it does not mark it as being available to be reused.
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

#include <stdlib.h>
#include <apr_pools.h>
#include <apr_lib.h>

#include "str_buf.h"
#include "misc.h"

#define DEFAULT_BUF_SIZE 256

/* Make sure we can append len bytes. */
#define CHECK_SIZE( buf, len )                                          \
  if ( ( buf->data_len + len ) > buf->data_size ) {                     \
    apr_pool_cleanup_kill( buf->mp, buf->data, mem_free );              \
    while ( buf->data_size < buf->data_len + len ) {                    \
      buf->data_size = buf->data_size * 2;                              \
    }                                                                   \
    buf->data = realloc( buf->data, buf->data_size );                   \
    apr_pool_cleanup_register( buf->mp, buf->data, mem_free,            \
                               apr_pool_cleanup_null );                 \
  }

str_buf_t *str_buf_create( apr_pool_t *mp, apr_size_t initial_size )
{
  str_buf_t *buf = apr_palloc( mp, sizeof(str_buf_t) );

  initial_size = ( initial_size <= 0 ) ? DEFAULT_BUF_SIZE : initial_size;

  buf->mp = mp;
  buf->data = malloc( initial_size );
  buf->data_len = 0;
  buf->data_size = initial_size;

  /*
   * Register a cleanup so that this can properly be freed when the pool is
   * destroyed.
   */
  apr_pool_cleanup_register( mp, buf->data, mem_free, apr_pool_cleanup_null );

  return buf;
}

void str_buf_putc( str_buf_t *buf, char c )
{
  CHECK_SIZE( buf, 1 );
  buf->data[buf->data_len++] = c;
}

void str_buf_append( str_buf_t *buf, const char *str )
{
  int len = strlen( str );
  CHECK_SIZE( buf, len );
  memcpy( buf->data + buf->data_len, str, len );
  buf->data_len += len;
}

void str_buf_write( str_buf_t *buf, const char *data, apr_size_t len )
{
  CHECK_SIZE( buf, len );
  memcpy( &buf->data[buf->data_len], data, len );
  buf->data_len += len;
}

typedef struct str_buf_printf_data {
  apr_vformatter_buff_t vbuf;
  char *buf;
  str_buf_t *str_buf;
}str_buf_printf_data;

static int str_buf_flush( apr_vformatter_buff_t *buf )
{
  str_buf_printf_data *data = (str_buf_printf_data *) buf;
  str_buf_write( data->str_buf, data->buf, data->vbuf.curpos - data->buf );
  data->vbuf.curpos = data->buf;

  return 0;
}

void str_buf_vprintf( str_buf_t *buf, char *format, va_list args )
{
  str_buf_printf_data data;
  char tmp_buf[4096];
  int count;

  data.buf = tmp_buf;
  data.vbuf.curpos = data.buf;
  data.vbuf.endpos = data.buf + 4096;
  data.str_buf = buf;

  count = apr_vformatter( str_buf_flush, 
                          (apr_vformatter_buff_t *) &data, format, args );
  if ( count >= 0 ) {
    str_buf_flush( (apr_vformatter_buff_t *) &data );
  }
}

void str_buf_printf( str_buf_t *buf, char *format, ... )
{
  va_list args;

  va_start( args, format );
  str_buf_vprintf( buf, format, args );
  va_end( args );
}
