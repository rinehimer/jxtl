#ifndef STR_BUF_H
#define STR_BUF_H

#include <apr_pools.h>

typedef struct str_buf_t {
  /** Amount used. */
  int data_len;
  /** Amount allocated. */
  int data_size;
  /** The actual buffer. */
  unsigned char *data;
}str_buf_t;

str_buf_t *str_buf_create( apr_pool_t *mp, int initial_size );
void str_buf_putc( str_buf_t *buf, unsigned char c );
void str_buf_append_str( str_buf_t *buf, const unsigned char *str );
void str_buf_append_strn( str_buf_t *buf, const unsigned char *str, int len );

#define STR_BUF_CLEAR( buf ) buf->data_len = 0;

#endif
