#include <stdlib.h>
#include <apr_pools.h>

#include "str_buf.h"

#define DEFAULT_BUF_SIZE 256

/* Make sure we can append len bytes. */
#define CHECK_SIZE( buf, len )                          \
  if ( ( buf->data_len + len ) > buf->data_size ) {     \
    buf->data_size = buf->data_size * 2;                \
    buf->data = realloc( buf->data, buf->data_size );   \
  }

str_buf_t *str_buf_create( apr_pool_t *mp, int initial_size )
{
  str_buf_t *buf = apr_palloc( mp, sizeof(str_buf_t) );

  initial_size = ( initial_size <= 0 ) ? DEFAULT_BUF_SIZE : initial_size;

  buf->data = malloc( sizeof(initial_size) );
  buf->data_len = 0;
  buf->data_size = initial_size;

  /*
   * Register a cleanup so that this can properly be freed when the pool is
   * destroyed.
   */
  apr_pool_cleanup_register( mp, buf->data, free, apr_pool_cleanup_null );

  return buf;
}

void str_buf_putc( str_buf_t *buf, unsigned char c )
{
  CHECK_SIZE( buf, 1 );
  buf->data[buf->data_len++] = c;
}

void str_buf_append_str( str_buf_t *buf, const unsigned char *str )
{
  int len = strlen( (char *) str );
  CHECK_SIZE( buf, len );
  memcpy( buf->data + buf->data_len, str, len );
  buf->data_len += len;
}

void str_buf_append_strn( str_buf_t *buf, const unsigned char *str, int len )
{
  CHECK_SIZE( buf, len );
  memcpy( buf->data + buf->data_len, str, len );
  buf->data_len += len;
}
