/*
 * json_verify.c
 *
 * Description
 *  Parse a JSON file and set a return status.
 *  This program will exit with status 0 if parsing was successful
 *  and 1 if there was an error.
 *
 * Copyright 2017 Dan Rinehimer
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

#include <apr_general.h>
#include <apr_getopt.h>
#include <stdlib.h>

#include "apr_macros.h"
#include "json.h"
#include "parser.h"
#include "misc.h"

/**
 * Read in the command line arguments to get the JSON file.
 */
void json_verify_init( int argc, char const * const *argv, apr_pool_t *mp,
                       const char **json_file )
{
  apr_getopt_t *options;
  apr_status_t ret;
  int ch;
  const char *arg;
  const apr_getopt_option_t json_verify_options[] = {
    { "json", 'j', 1, "JSON file to verify" },
    { 0, 0, 0, 0 }
  };

  *json_file = NULL;

  apr_getopt_init( &options, mp, argc, argv );

  while ( ( ret = apr_getopt_long( options, json_verify_options, &ch,
                                   &arg ) ) == APR_SUCCESS ) {
    switch ( ch ) {
    case 'j':
      *json_file = arg;
      break;
    }
  }

  if ( ( ret == APR_BADCH )  || ( *json_file == NULL ) ) {
    print_usage( argv[0], json_verify_options );
    exit( EXIT_FAILURE );
  }
}

int main( int argc, char const * const *argv )
{
  apr_pool_t *mp;
  const char *json_filepath = NULL;
  apr_file_t *json_file;
  parser_t *json_parser;
  int status = 0;

  apr_app_initialize( NULL, NULL, NULL );
  apr_pool_create( &mp, NULL );

  json_verify_init( argc, argv, mp, &json_filepath );
  json_parser = json_parser_create( mp );

  if ( !open_apr_input_file( mp, json_filepath, &json_file ) ) {
    status = 1;
  }
  else if ( !json_parser_parse_file( json_parser, (const void *) json_file,
                                     NULL ) ) {
    status = 1;
  }
  else {
    status = 0;
  }

  apr_pool_destroy( mp );
  apr_terminate();

  return status;
}
