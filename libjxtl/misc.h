/*
 * misc.h
 *
 * Description
 *   Any miscellaneous library routines that don't really fit anywhere else.
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
#include <apr_file_io.h>
#include <apr_pools.h>
#include <apr_file_io.h>
#include <apr_getopt.h>

/**
 * Function that just calls free and returns APR_SUCCESS.
 */
apr_status_t mem_free( void *ptr );

/**
 * Wrapper function that opens stdin if file_name is "-".
 */
int open_apr_input_file( apr_pool_t *mp, const char *file_name,
                         apr_file_t **file );
int open_apr_output_file( apr_pool_t *mp, const char *file_name,
                          apr_file_t **file );

/**
 * Generic function to print the usage of program based on its options.
 */
void print_usage( const char *prog_name,
                  const apr_getopt_option_t *options );
