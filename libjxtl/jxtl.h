/*
 * jxtl.h
 *
 * Description
 *   API for the parsing and expanding templates.
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

#ifndef JXTL_H
#define JXTL_H

#include <apr_pools.h>

#include "parser.h"

/**
 * Structure that holds the callbacks functions for the jxtl parser.
 */
typedef struct jxtl_callback_t {
  void ( *text_handler )( void *user_data, unsigned char *text );
  int ( *section_start_handler )( void *user_data, unsigned char *expr );
  void ( *section_end_handler )( void *user_data );
  int ( *if_start_handler )( void *user_data, unsigned char *expr );
  int ( *elseif_handler )( void *user_data, unsigned char *expr );
  void ( *else_handler )( void *user_data );
  void ( *if_end_handler )( void *user_data );
  void ( *separator_start_handler )( void *user_data );
  void ( *separator_end_handler )( void *user_data );
  int ( *value_handler )( void *user_data, unsigned char *expr );
  void ( *format_handler )( void *user_data, char *format );
  char * ( *get_error_func )( void *user_data );
  void *user_data;
} jxtl_callback_t;

parser_t *jxtl_parser_create( apr_pool_t *mp );
int jxtl_parser_parse_file( parser_t *parser, const char *file,
                            jxtl_callback_t *callbacks );
int jxtl_parser_parse_buffer( parser_t *parser, const char *buffer,
                              jxtl_callback_t *callbacks );

#endif
