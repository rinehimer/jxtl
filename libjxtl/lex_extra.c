#include <apr_file_info.h>
#include <apr_file_io.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>
#include <stdlib.h>

#include "apr_macros.h"
#include "lex_extra.h"

lex_extra_t *create_lex_extra( apr_pool_t *mp, const char *filename )
{
  lex_extra_t *lex_extra = apr_palloc( mp, sizeof( lex_extra_t ) );
  apr_status_t status;
  apr_file_t *in_file = NULL;
  apr_finfo_t file_info;
  char error_buf[1024];

  if ( filename && apr_strnatcasecmp( filename, "stdin" ) != 0 ) {
    status = apr_file_open( &in_file, filename, APR_READ | APR_BUFFERED, 0,
			    mp );

    if ( status != APR_SUCCESS ) {
      apr_strerror( status, error_buf, 1024 );
      fprintf( stderr, "%s\n", error_buf );
      exit( EXIT_FAILURE );
    }
  }
  else if ( filename && apr_strnatcasecmp( filename, "stdin" ) == 0 ) {
    status = apr_file_open_stdin( &in_file, mp );

    if ( status != APR_SUCCESS ) {
      apr_strerror( status, error_buf, 1024 );
      fprintf( stderr, "%s\n", error_buf );
      exit( EXIT_FAILURE );
    }
  }

  apr_pool_create( &lex_extra->mp, NULL );
  lex_extra->str_array = apr_array_make( lex_extra->mp, 8192, sizeof( char ) );
  lex_extra->in_file = in_file;

  return lex_extra;
}

void destroy_lex_extra( lex_extra_t *lex_extra )
{
  apr_pool_destroy( lex_extra->mp );
}
