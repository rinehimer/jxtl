/*
 * misc.c
 *
 * Description
 *   Contains the misc functions.
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
#include <apr_errno.h>
#include <apr_general.h>
#include <apr_getopt.h>
#include <apr_file_io.h>
#include <apr_pools.h>
#include <apr_strings.h>

apr_status_t mem_free( void *ptr )
{
  free( ptr );

  return APR_SUCCESS;
}

int open_apr_input_file( apr_pool_t *mp, const char *file_name,
                         apr_file_t **file )
{
  apr_status_t status;

  if ( apr_strnatcasecmp( file_name, "-" ) == 0 ) {
    status = apr_file_open_stdin( file, mp );
  }
  else {
    status = apr_file_open( file, file_name, APR_READ | APR_BUFFERED, 0, mp );
  }
  return ( status == APR_SUCCESS );
}

int open_apr_output_file( apr_pool_t *mp, const char *file_name,
                          apr_file_t **file )
{
  apr_status_t status;

  if ( !file_name || apr_strnatcasecmp( file_name, "-" ) == 0 ) {
    status = apr_file_open_stdout( file, mp );
  }
  else {
    status = apr_file_open( file, file_name, APR_WRITE | APR_CREATE |
                            APR_BUFFERED | APR_TRUNCATE, APR_OS_DEFAULT, mp );
  }
  return ( status == APR_SUCCESS );
}

void print_usage( const char *prog_name,
                  const apr_getopt_option_t *options )
{
  int i;

  printf( "Usage: %s [options]\n", prog_name );
  printf( "  Options:\n" );
  for ( i = 0; options[i].name; i++ ) {
    printf( "    -%c, --%s %s\n", options[i].optch, options[i].name,
            options[i].description );
  }
}
