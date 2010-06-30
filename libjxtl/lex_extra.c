#include <apr_file_info.h>
#include <apr_file_io.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>
#include <stdlib.h>

#include "apr_macros.h"
#include "lex_extra.h"

void lex_extra_init( lex_extra_t *lex_extra, const char *filename )
{
  apr_status_t status;
  apr_file_t *in_file = NULL;
  char error_buf[1024];

  apr_pool_create( &lex_extra->mp, NULL );

  if ( filename && apr_strnatcasecmp( filename, "stdin" ) != 0 ) {
    status = apr_file_open( &in_file, filename, APR_READ | APR_BUFFERED, 0,
			    lex_extra->mp );

    if ( status != APR_SUCCESS ) {
      apr_strerror( status, error_buf, 1024 );
      fprintf( stderr, "%s\n", error_buf );
      exit( EXIT_FAILURE );
    }
  }
  else if ( filename && apr_strnatcasecmp( filename, "stdin" ) == 0 ) {
    status = apr_file_open_stdin( &in_file, lex_extra->mp );

    if ( status != APR_SUCCESS ) {
      apr_strerror( status, error_buf, 1024 );
      fprintf( stderr, "%s\n", error_buf );
      exit( EXIT_FAILURE );
    }
  }

  lex_extra->str_array = apr_array_make( lex_extra->mp, 8192, sizeof( char ) );
  lex_extra->in_file = in_file;
}

void lex_extra_destroy( lex_extra_t *lex_extra )
{
  apr_pool_destroy( lex_extra->mp );
}
