/*
 * jxtl_template.h
 *
 * Description
 *   API for working with jxtl_templates.
 *
 * Copyright 2011 Dan Rinehimer
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

#ifndef JXTL_TEMPLATE_H
#define JXTL_TEMPLATE_H

#include <apr_buckets.h>
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_tables.h>

#include "jxtl_path_expr.h"

typedef enum jxtl_content_type {
  JXTL_TEXT,
  JXTL_SECTION,
  JXTL_VALUE,
  JXTL_IF,
  JXTL_PARAM_REF,
  JXTL_PARAM_DECL
} jxtl_content_type;

typedef struct jxtl_content_t {
  /**
   * What this content contains in its value pointer.
   */
  jxtl_content_type type;

  /**
   * A string, pointer to a jxtl_section_t, jxtl_if_t, jxtl_path_expr_t or
   * jxtl_param_t.
   */
  void *value;

  /**
   * Array of content for the separator.
   */
  apr_array_header_t *separator;

  /**
   * A format to be applied.
   */
  char *format;

  /**
   * Parameters declared within this content scope.
   */
  apr_hash_t *params;
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
 * Type that stores a parameter.
 */
typedef struct jxtl_param_t {
  char *name;
  apr_array_header_t *content;
} jxtl_param_t;

typedef apr_status_t ( *brigade_flush_func )( apr_bucket_brigade *bb,
                                              void *ctx );

typedef struct jxtl_template_t {
  apr_pool_t *expand_mp;
  apr_array_header_t *content;
  apr_bucket_brigade *bb;
  brigade_flush_func flush_func;
  void *flush_data;
  apr_hash_t *formats;
  void *format_data;
} jxtl_template_t;

typedef char * ( *jxtl_format_func )( json_t *value, char *format,
                                      void *user_data );

/**
 * Parse a file into a jxtl_template.
 */
int jxtl_parser_parse_file_to_template( apr_pool_t *mp, parser_t *parser,
                                        apr_file_t *file,
                                        jxtl_template_t **template );
/**
 * Parse a buffer into a jxtl_template.
 */
int jxtl_parser_parse_buffer_to_template( apr_pool_t* mp, parser_t *parser,
                                          const char *buffer,
                                          jxtl_template_t **template );

/**
 * Register a named format with a callback function.
 */
void jxtl_template_register_format( jxtl_template_t *template,
                                    const char *format_name,
                                    jxtl_format_func func );

/**
 * Set the user data to be passed into format callbacks.
 */
void jxtl_template_set_format_data( jxtl_template_t *template,
                                    void *format_data );

/**
 * Generic template expansion function.  This function is called by
 * jxtl_template_expand_to_file and jxtl_template_expand_to_buffer.
 */
void expand_template( jxtl_template_t *template, json_t *json,
                      brigade_flush_func flush_func, void *flush_data );

/**
 * Expand a template to a file.
 */
int jxtl_template_expand_to_file( jxtl_template_t *template, json_t *json,
                                  apr_file_t *file );

/**
 * Expand a template into a buffer that is allocated from mp.
 */
char *jxtl_template_expand_to_buffer( apr_pool_t *user_mp,
                                      jxtl_template_t *template,
                                      json_t *json );

#endif
