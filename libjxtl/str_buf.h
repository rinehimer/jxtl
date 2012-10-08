/*
 * str_buf.h
 *
 * Description
 *   API for the dynamic string buffer.
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

#ifndef STR_BUF_H
#define STR_BUF_H

#include <apr_pools.h>

typedef struct str_buf_t {
  /** Pool used to to allocate this object. */
  apr_pool_t *mp;
  /** Amount used. */
  int data_len;
  /** Amount allocated. */
  int data_size;
  /** The actual buffer. */
  char *data;
}str_buf_t;

/**
 * Create a new string buffer.  This buffer will be properly cleaned up when
 * the memory pool it is allocated from is cleared or destroyed.
 * @param mp The memory pool to allocate the buffer out of.
 * @param initial_size The initial size to make the buffer.  If this value is
 *        <= 0 then a default size is used.
 * @return The newly allocated string buffer.
 */
str_buf_t *str_buf_create( apr_pool_t *mp, apr_size_t initial_size );

/**
 * Write a character to the str_buf.
 * @param buf The string buffer.
 * @param c The character to write.
 */
void str_buf_putc( str_buf_t *buf, char c );

/**
 * Append a string to the str_buf.
 * @param buf The string buffer.
 * @param str The string to append.
 */
void str_buf_append( str_buf_t *buf, const char *str );

/**
 * Write data to the end of the str_buf.
 * @param buf The string buffer.
 * @param data The data to write.
 * @param len The length of the data.
 */
void str_buf_write( str_buf_t *buf, const char *data, apr_size_t len );

/**
 * Write to a string buf using a va_list.
 * @param buf The string buffer.
 * @param format The string format.
 * @param args An arg list.
 */
void str_buf_vprintf( str_buf_t *buf, const char *format, va_list args );

/**
 * Write to a buffer using a printf style of arguments.
 * @param buf The string buffer.
 * @param format The string format.
 * @param ... Arguments to use in the format.
 */
void str_buf_printf( str_buf_t *buf, const char *format, ... );

#define STR_BUF_CLEAR( buf ) buf->data_len = 0

#endif
