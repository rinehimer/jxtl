/*
 * $Id$
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
  unsigned char *data;
}str_buf_t;

str_buf_t *str_buf_create( apr_pool_t *mp, apr_size_t initial_size );
void str_buf_putc( str_buf_t *buf, char c );
void str_buf_append( str_buf_t *buf, const char *str );
void str_buf_write( str_buf_t *buf, const char *data, apr_size_t len );

#define STR_BUF_CLEAR( buf ) buf->data_len = 0

#endif
