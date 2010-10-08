#ifndef JXTL_H
#define JXTL_H

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
  /** Array of content for the separator. */
  apr_array_header_t *separator;
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
  void *user_data;
} jxtl_callback_t;

typedef char * ( *jxtl_format_func )( char *value, char *format_name,
                                      void *user_data );

typedef struct jxtl_template_t {
  apr_array_header_t *content;
  void *format_data;
  jxtl_format_func format;
}jxtl_template_t;

parser_t *jxtl_parser_create( apr_pool_t *mp );
int jxtl_parser_parse_file( parser_t *parser, const char *file,
                            jxtl_template_t **template );
int jxtl_parser_parse_buffer( parser_t *parser, const char *buffer,
                              jxtl_template_t **template );

void jxtl_template_set_format_func( jxtl_template_t *template,
                                    jxtl_format_func format_func );

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
