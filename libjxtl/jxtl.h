/*
 * $Id$
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

#include <apr_buckets.h>
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_tables.h>

#include "parser.h"
#include "json.h"
#include "jxtl_path.h"

typedef enum jxtl_content_type {
  JXTL_TEXT,
  JXTL_SECTION,
  JXTL_VALUE,
  JXTL_IF
} jxtl_content_type;

typedef struct jxtl_content_t {
  /** What this content contains in its value pointer. */
  jxtl_content_type type;
  /** A string, pointer to a jxtl_section_t, jxtl_if_t or a jxtl_path_expr_t */
  void *value;
  /** Array of content for the separator. */
  apr_array_header_t *separator;
  /** A format to be applied. */
  char *format;
} jxtl_content_t;

typedef struct jxtl_if_t {
  jxtl_path_expr_t *expr;
  apr_array_header_t *content;
} jxtl_if_t;

typedef struct jxtl_section_t {
  /** Compiled path expression. */
  jxtl_path_expr_t *expr;
  /** Array of the content in the section. */
  apr_array_header_t *content;
} jxtl_section_t;

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
  int own_user_data;
  void *user_data;
} jxtl_callback_t;

typedef char * ( *jxtl_format_func )( json_t *value, char *format,
                                      void *user_data );

typedef struct jxtl_template_t {
  apr_array_header_t *content;
  apr_status_t ( *flush_func )( apr_bucket_brigade *bb, void *ctx );
  void *flush_data;
  apr_hash_t *formats;
  void *format_data;
}jxtl_template_t;

parser_t *jxtl_parser_create( apr_pool_t *mp, jxtl_callback_t *user_callbacks );
int jxtl_parser_parse_file( parser_t *parser, const char *file,
                            jxtl_template_t **template );
int jxtl_parser_parse_buffer( parser_t *parser, const char *buffer,
                              jxtl_template_t **template );

/**
 * Register a named format with a callback function.
 */
void jxtl_template_register_format( jxtl_template_t *template,
                                    const char *format_name,
                                    jxtl_format_func func );

void jxtl_template_set_format_data( jxtl_template_t *template,
                                    void *format_data );

/**
 * Expand a template to a named file.
 */
int jxtl_expand_to_file( jxtl_template_t *template, json_t *json,
                         const char *file );

/**
 * Expand a template into a buffer that is allocated from mp.
 */
char *jxtl_expand_to_buffer( apr_pool_t *mp, jxtl_template_t *template,
                             json_t *json );

#endif
