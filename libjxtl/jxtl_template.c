/*
 * jxtl_template.c
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

#include <apr_buckets.h>
#include <apr_hash.h>
#include <apr_general.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "jxtl.h"
#include "jxtl_path.h"
#include "jxtl_path_expr.h"
#include "jxtl_template.h"
#include "json.h"
#include "str_buf.h"

/**
 * Structure to hold data during parsing.  One of these will be passed to the
 * callback functions.
 */
typedef struct jxtl_data_t {
  /**
   * Memory pool used to allocate all objects during parsing.
   */
  apr_pool_t *mp;

  /**
   * Pointer to the current content array.
   */
  apr_array_header_t *current_array;

  /**
   * Stack of our content arrays.
   */
  apr_array_header_t *content_array;

  /**
   * Pointer to the last section or value pushed on.  This pointer only matters
   * when we encounter a separator.
   */
  jxtl_content_t *last_section_or_value;

  /**
   * Pointer to the last section pushed into the content.
   */
  jxtl_section_t *last_section;

  /**
   * Reusable path parser.
   */
  parser_t *jxtl_path_parser;

  /**
   * Reusable parser when we need to parse a quoted string.
   */
  parser_t *jxtl_parser;

  /**
   * Last error encountered during parsing.  Could come from the path
   * parser or be something like an invalid variable reference.
   */
  str_buf_t *error_buf;
} jxtl_data_t;

/*
 * Convenience function to create a new content object and it on the current
 * array.
 */
static void content_push( jxtl_data_t *data, jxtl_content_type type,
                          void *value )
{
  jxtl_content_t *content = NULL;

  content = apr_palloc( data->mp, sizeof(jxtl_content_t) );
  content->type = type;
  content->value = value;
  content->separator = NULL;
  content->format = NULL;

  if ( ( type == JXTL_SECTION ) || ( type == JXTL_VALUE ) ) {
    data->last_section_or_value = content;
  }

  APR_ARRAY_PUSH( data->current_array, jxtl_content_t * ) = content;
}

/**
 * Update the current content array.  We push the existing current_array
 * onto our stack and then update the pointer.
 */
static void set_content_array( jxtl_data_t *data,
                                apr_array_header_t *content_array )
{
  /* Save off the current one */
  APR_ARRAY_PUSH( data->content_array,
                  apr_array_header_t * ) = data->current_array;

  /* Update the pointer */
  data->current_array = content_array;;
}

static jxtl_content_t *jxtl_get_last_content( jxtl_data_t *data )
{
  apr_array_header_t *content_array;
  jxtl_content_t *content;

  content_array = APR_ARRAY_TAIL( data->content_array, apr_array_header_t * );
  return APR_ARRAY_TAIL( content_array, jxtl_content_t * );
}

static void jxtl_set_error( jxtl_data_t *data, const char *error_string, ... )
{
  va_list args;

  STR_BUF_CLEAR( data->error_buf );

  va_start( args, error_string );
  str_buf_vprintf( data->error_buf, error_string, args );
  STR_BUF_NULL_TERMINATE( data->error_buf );
  va_end( args );
}

/*****************************************************************************
 * Parser callbacks
 *****************************************************************************/

/**
 * Parser callback for when it finds text.
 * @param user_data The jxtl_data.
 * @param text The text.
 */
static void jxtl_text_handler( void *user_data, char *text )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  content_push( data, JXTL_TEXT, apr_pstrdup( data->mp, text ) );
}

/**
 * Parser callback for starting a section.  We need to allocate a new section,
 * parse the expression and then add it to our content array.  We also need to
 * make sure the current_array points to the one in the new section object so 
 * that content encountered after the section is properly added to it.
 * @param user_data The jxtl_data
 * @param expr An expression to evaluate for the section
 */
static int jxtl_section_start( void *user_data, char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_section_t *section;
  int result;

  section = apr_palloc( data->mp, sizeof(jxtl_section_t) );
  result = jxtl_path_parser_parse_buffer_to_expr( data->mp,
                                                  data->jxtl_path_parser,
                                                  expr,
                                                  &section->expr );
  if ( !result ) {
    jxtl_set_error( data, parser_get_error( data->jxtl_path_parser ) );
  }

  section->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  section->vars = apr_hash_make( data->mp );
  data->last_section = section;
  content_push( data, JXTL_SECTION, section );
  set_content_array( data, section->content );

  return result;
}

/**
 * Parser callback for when a section ends.  All we need to do is reset the
 * current_array to the previous item on the content_array stack.
 * @param user_data The jxtl_data.
 */
static void jxtl_section_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;

  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
}

static int jxtl_var_decl( void *user_data, char *name, char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_content_t *content;
  jxtl_var_t *var;
  jxtl_template_t *template;
  int result;

  var = apr_palloc( data->mp, sizeof(jxtl_var_t) );
  var->name = apr_pstrdup( data->mp, name );
  result = jxtl_path_parser_parse_buffer_to_expr( data->mp,
                                                  data->jxtl_path_parser,
                                                  expr, &var->expr );
  if ( !result ) {
    jxtl_set_error( data, parser_get_error( data->jxtl_path_parser ) );
  }
  
  apr_hash_set( data->last_section->vars, var->name, APR_HASH_KEY_STRING, var );

  return result;
}

/**
 * Parser callback for starting an if statement.
 * @param user_data The jxtl_data
 * @param expr An expression to evaluate for the if
 */
static int jxtl_if_start( void *user_data, char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_if_t *jxtl_if;
  apr_array_header_t *if_block;
  int result;

  if_block = apr_array_make( data->mp, 8, sizeof(jxtl_if_t *) );
  jxtl_if = apr_palloc( data->mp, sizeof(jxtl_if_t) );
  result = jxtl_path_parser_parse_buffer_to_expr( data->mp,
                                                  data->jxtl_path_parser,
                                                  expr,
                                                  &jxtl_if->expr );
  if ( !result ) {
    jxtl_set_error( data, parser_get_error( data->jxtl_path_parser ) );
  }

  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  content_push( data, JXTL_IF, if_block );
  set_content_array( data, jxtl_if->content );

  return result;
}

/**
 * Parser callback for starting an elseif statement.  A little different than
 * starting an if because we need to get the if_block array and add the new
 * jxtl_if_t object to that.
 * @param user_data The jxtl_data
 * @param expr An expression to evaluate for the elseif
 */
static int jxtl_elseif( void *user_data, char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  apr_array_header_t *if_block;
  jxtl_if_t *jxtl_if;
  jxtl_content_t *content;
  int result;

  content = jxtl_get_last_content( data );
  if_block = (apr_array_header_t *) content->value;
  jxtl_if = apr_palloc( data->mp, sizeof(jxtl_if_t) );
  result = jxtl_path_parser_parse_buffer_to_expr( data->mp, 
                                                  data->jxtl_path_parser,
                                                  expr,
                                                  &jxtl_if->expr );
  if ( !result ) {
    jxtl_set_error( data, parser_get_error( data->jxtl_path_parser ) );
  }

  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  data->current_array = jxtl_if->content;

  return result;
}

static void jxtl_else( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  apr_array_header_t *if_block;
  jxtl_if_t *jxtl_if;
  jxtl_content_t *content;

  content = jxtl_get_last_content( data );
  if_block = (apr_array_header_t *) content->value;

  jxtl_if = apr_palloc( data->mp, sizeof(jxtl_if_t) );
  jxtl_if->expr = NULL;
  jxtl_if->content = apr_array_make( data->mp, 1024,
                                     sizeof(jxtl_content_t *) );
  APR_ARRAY_PUSH( if_block, jxtl_if_t * ) = jxtl_if;
  data->current_array = jxtl_if->content;
}

static void jxtl_if_end( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;

  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
}

/**
 * Parser callback for when it encounters a separator option.  This funciton
 * is currently bastardized.  Really what should happen here is that the
 * expr should be recursively parsed.
 */
static void jxtl_separator_handler( void *user_data, char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_content_t *content = data->last_section_or_value;

  content->separator = apr_array_make( data->mp, 1, sizeof(jxtl_content_t *) );
  set_content_array( data, content->separator );
  jxtl_text_handler( data, expr );
  data->current_array = APR_ARRAY_POP( data->content_array,
                                       apr_array_header_t * );
}

/**
 * Parser callback function for when it encounters a value reference in the
 * template, i.e. {{value}}. The expression is parsed and the result saved off.
 * @param user_data The jxtl_data.
 * @param expr The value expression to parse.
 */
static int jxtl_value_handler( void *user_data, char *expr )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  jxtl_path_expr_t *path_expr;
  int result;

  result = jxtl_path_parser_parse_buffer_to_expr( data->mp,
                                                  data->jxtl_path_parser,
                                                  expr,
                                                  &path_expr );
  if ( !result ) {
    jxtl_set_error( data, parser_get_error( data->jxtl_path_parser ) );
  }

  content_push( data, JXTL_VALUE, path_expr );

  return result;
}

static char *jxtl_get_error( void *user_data )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  return data->error_buf->data;
}

static void jxtl_format_handler( void *user_data, char *format )
{
  jxtl_data_t *data = (jxtl_data_t *) user_data;
  data->last_section_or_value->format = apr_pstrdup( data->mp, format );
}

static jxtl_template_t *jxtl_template_create( apr_pool_t *mp,
                                              apr_array_header_t *content )
{
  jxtl_template_t *template;
  template = apr_palloc( mp, sizeof(jxtl_template_t) );
  apr_pool_create( &template->expand_mp, NULL );
  template->content = content;
  template->flush_func = NULL;
  template->flush_data = NULL;
  template->formats = apr_hash_make( mp );
  template->format_data = NULL;

  return template;
}

static void initialize_callbacks( apr_pool_t *template_mp, 
                                  apr_pool_t *tmp_mp,
                                  jxtl_callback_t *callbacks,
                                  jxtl_data_t *cb_data )
{
  apr_array_header_t *initial_array;

  callbacks->text_handler = jxtl_text_handler;
  callbacks->section_start_handler = jxtl_section_start;
  callbacks->section_end_handler = jxtl_section_end;
  callbacks->var_decl_handler = jxtl_var_decl;
  callbacks->if_start_handler = jxtl_if_start;
  callbacks->elseif_handler = jxtl_elseif;
  callbacks->else_handler = jxtl_else;
  callbacks->if_end_handler = jxtl_if_end;
  callbacks->separator_handler = jxtl_separator_handler;
  callbacks->value_handler = jxtl_value_handler;
  callbacks->get_error_func = jxtl_get_error;
  callbacks->format_handler = jxtl_format_handler;

  cb_data->mp = template_mp;
  cb_data->content_array = apr_array_make( tmp_mp, 1024,
                                           sizeof(apr_array_header_t *) );
  initial_array = apr_array_make( cb_data->mp, 1024,
                                  sizeof(apr_array_header_t *) );
  APR_ARRAY_PUSH( cb_data->content_array,
                  apr_array_header_t * ) = initial_array;
  cb_data->current_array = initial_array;
  cb_data->jxtl_path_parser = jxtl_path_parser_create( tmp_mp );
  cb_data->jxtl_parser = jxtl_parser_create( tmp_mp );
  cb_data->error_buf = str_buf_create( tmp_mp, 512 );

  callbacks->user_data = cb_data;
}

int jxtl_parser_parse_file_to_template( apr_pool_t *mp, parser_t *parser,
                                        apr_file_t *file,
                                        jxtl_template_t **template )
{
  int result = FALSE;
  apr_pool_t *tmp_mp;
  jxtl_callback_t callbacks;
  jxtl_data_t callback_data;

  apr_pool_create( &tmp_mp, NULL );
  initialize_callbacks( mp, tmp_mp, &callbacks, &callback_data );
  *template = NULL;

  if ( jxtl_parser_parse_file( parser, file, &callbacks ) ) {
    *template = jxtl_template_create( mp, callback_data.current_array );
    result = TRUE;
  }

  apr_pool_destroy( tmp_mp );
  return result;
}

int jxtl_parser_parse_buffer_to_template( apr_pool_t* mp, parser_t *parser,
                                          const char *buffer,
                                          jxtl_template_t **template )
{
  int result = FALSE;
  apr_pool_t *tmp_mp;
  jxtl_callback_t callbacks;
  jxtl_data_t callback_data;

  apr_pool_create( &tmp_mp, NULL );
  initialize_callbacks( mp, tmp_mp, &callbacks, &callback_data );
  *template = NULL;

  if ( jxtl_parser_parse_buffer( parser, buffer, &callbacks ) ) {
    *template = jxtl_template_create( mp, callback_data.current_array );
    result = TRUE;
  }

  apr_pool_destroy( tmp_mp );
  return result;
}

/***************************************************************************
  Expansion definitions and functions
 ***************************************************************************/

/** Constants used for calling the expand functions */
typedef enum section_print_type {
  PRINT_NORMAL,
  PRINT_SECTION,
  PRINT_SEPARATOR
} section_print_type;

static apr_status_t flush_to_file( apr_bucket_brigade *bb, void *ctx )
{
  apr_bucket *e;
  const char *str;
  apr_size_t str_len;
  apr_file_t *out = (apr_file_t *) ctx;

  for ( e = APR_BRIGADE_FIRST( bb );
        e != APR_BRIGADE_SENTINEL( bb );
        e = APR_BUCKET_NEXT( e ) ) {
    apr_bucket_read( e, &str, &str_len, APR_BLOCK_READ );
    apr_file_write( out, str, &str_len );
  }

  apr_brigade_cleanup( bb );

  return APR_SUCCESS;
}

static void print_json_value( json_t *json,
                              char *format,
                              apr_pool_t *mp,
                              jxtl_template_t *template )
{
  char *value = NULL;
  jxtl_format_func format_func = NULL;

  if ( !json )
    return;

  if ( format ) {
    format_func = apr_hash_get( template->formats, format,
                                APR_HASH_KEY_STRING );
  }

  if ( format_func ) {
    value = format_func( json, format, template->format_data );
  }
  else {
    value = json_get_string_value( mp, json );
  }

  if ( value ) {
    apr_brigade_printf( template->bb, template->flush_func,
                        template->flush_data, "%s", value );
  }
}

static void print_text( char *text,
                        jxtl_content_t *prev_content,
                        jxtl_content_t *next_content,
                        section_print_type print_type,
                        jxtl_template_t *template )
{
  char *text_ptr = text;
  int len = strlen( text_ptr );

  if ( ( print_type == PRINT_SECTION ) && ( !prev_content ) &&
       ( text_ptr[0] == '\n' ) ) {
    text_ptr++;
    len--;
  }
  if ( ( print_type == PRINT_SECTION ) && ( !next_content ) &&
       ( text_ptr[len - 1] == '\n' ) ) {
    len--;
  }
  if ( len > 0 ) {
    apr_brigade_printf( template->bb, template->flush_func,
                        template->flush_data, "%.*s", len, text_ptr );
  }
}

static void expand_content( apr_pool_t *mp,
                            jxtl_template_t *template,
                            apr_array_header_t *content_array,
                            json_t *json,
                            apr_hash_t *vars,
                            char *prev_format,
                            section_print_type print_type );

/**
 * Print a saved section
 */
static void expand_section( apr_pool_t *mp,
                            jxtl_template_t *template,
                            jxtl_section_t *section,
                            apr_array_header_t *separator,
                            json_t *json,
                            apr_hash_t *vars,
                            char *format,
                            section_print_type print_type )
{
  int i;
  int num_items;
  json_t *json_value;
  jxtl_path_obj_t *path_obj;

  if ( !json )
    return;

  num_items = jxtl_path_compiled_eval( mp, section->expr, json, vars,
                                       &path_obj );
  for ( i = 0; i < path_obj->nodes->nelts; i++ ) {
    json_value = APR_ARRAY_IDX( path_obj->nodes, i, json_t * );
    expand_content( mp, template, section->content, json_value, section->vars,
                    format, PRINT_SECTION );
    /* Only print the separator if it's not the last one */
    if ( separator && ( i + 1 < num_items ) ) {
      expand_content( mp, template, separator, json_value, section->vars,
                      format, PRINT_SEPARATOR );
    }
  }
}

/**
 * Determine if the result of an if statement is true.  The criteria is:
 * 1) More than 1 node returned is automatically true.
 * 2) If there was exactly one node and it's not a boolean then it's true.
 * 3) If there was exactly one node and it is a boolean and it's value is true.
 * 4) Anything else is false.
 */
static int is_true_if( apr_pool_t *mp, jxtl_if_t *jxtl_if, json_t *json,
                       apr_hash_t *vars )
{
  json_t *tmp_json;
  int result = FALSE;
  jxtl_path_obj_t *path_obj;

  jxtl_path_compiled_eval( mp, jxtl_if->expr, json, vars, &path_obj );

  if ( path_obj->nodes->nelts > 1 ) {
    result = TRUE;
  }
  else if ( path_obj->nodes->nelts == 1 ) {
    tmp_json = APR_ARRAY_HEAD( path_obj->nodes, json_t * );
    result = ( !JSON_IS_BOOLEAN( tmp_json ) ||
               JSON_IS_TRUE_BOOLEAN( tmp_json ) );
  }

  return jxtl_if->expr->negate ? !result : result;
}

static void expand_content( apr_pool_t *mp,
                            jxtl_template_t *template,
                            apr_array_header_t *content_array,
                            json_t *json,
                            apr_hash_t *vars,
                            char *prev_format,
                            section_print_type print_type )
{
  int i, j;
  jxtl_content_t *content, *prev_content, *next_content;
  jxtl_section_t *tmp_section;
  json_t *json_value;
  jxtl_if_t *jxtl_if;
  apr_array_header_t *if_block;
  jxtl_path_obj_t *path_obj;
  jxtl_var_t *var;
  char *format;

  prev_content = NULL;
  next_content = NULL;

  for ( i = 0; i < content_array->nelts; i++ ) {
    content = APR_ARRAY_IDX( content_array, i, jxtl_content_t * );
    next_content = ( i + 1 < content_array->nelts ) ?
                   APR_ARRAY_IDX( content_array, i + 1, jxtl_content_t * ) :
                   NULL;
    switch ( content->type ) {
    case JXTL_TEXT:
      print_text( content->value, prev_content, next_content, print_type,
                  template );
      break;

    case JXTL_SECTION:
      tmp_section = (jxtl_section_t *) content->value;
      format = ( content->format ) ? content->format : prev_format;
      expand_section( mp, template, tmp_section, content->separator, json, vars,
                      format, PRINT_SECTION );
      break;
      
    case JXTL_IF:
      /*
       * Loop through all of the ifs until we find a true one and then break
       * the loop.
       */
      if_block = (apr_array_header_t *) content->value;
      for ( j = 0; j < if_block->nelts; j++ ) {
        jxtl_if = APR_ARRAY_IDX( if_block, j, jxtl_if_t * );
        if ( !jxtl_if->expr || ( is_true_if( mp, jxtl_if, json, vars ) ) ) {
          expand_content( mp, template, jxtl_if->content, json, vars,
                          prev_format, PRINT_SECTION );
          break;
        }
      }
      break;
      
    case JXTL_VALUE:
      format = ( content->format ) ? content->format : prev_format;
      if ( jxtl_path_compiled_eval( mp, content->value, json, vars,
                                    &path_obj ) ) {
        for ( j = 0; j < path_obj->nodes->nelts; j++ ) {
          json_value = APR_ARRAY_IDX( path_obj->nodes, j, json_t * );
          print_json_value( json_value, format, mp, template );
          if ( content->separator && ( j + 1 < path_obj->nodes->nelts ) ) {
            expand_content( mp, template, content->separator, json_value,
                            vars, format, PRINT_SEPARATOR );
          }
        }
      }
      break;
    }
    prev_content = content;
  }
}

void jxtl_template_register_format( jxtl_template_t *template,
                                    const char *format_name,
                                    jxtl_format_func func )
{
  apr_hash_set( template->formats, format_name, APR_HASH_KEY_STRING, func );
}

void jxtl_template_set_format_data( jxtl_template_t *template,
                                    void *format_data )
{
  template->format_data = format_data;
}

void expand_template( jxtl_template_t *template, json_t *json,
                      brigade_flush_func flush_func, void *flush_data )
{
  apr_bucket_alloc_t *bucket_alloc;

  apr_pool_clear( template->expand_mp );

  template->flush_func = flush_func;
  template->flush_data = flush_data;
  bucket_alloc = apr_bucket_alloc_create( template->expand_mp );
  template->bb = apr_brigade_create( template->expand_mp, bucket_alloc );

  expand_content( template->expand_mp, template, template->content, json,
                  NULL, NULL, PRINT_NORMAL );
}
 
int jxtl_template_expand_to_file( jxtl_template_t *template, json_t *json,
                                  apr_file_t *out )
{
  expand_template( template, json, flush_to_file, out );
  flush_to_file( template->bb, out );
  return APR_SUCCESS;
}

char *jxtl_template_expand_to_buffer( apr_pool_t *user_mp,
                                      jxtl_template_t *template,
                                      json_t *json )
{
  char *expanded_template;
  apr_off_t length;
  apr_size_t flatten_len;

  expand_template( template, json, NULL, NULL );
  apr_brigade_length( template->bb, 1, &length );
  flatten_len = length;

  /* Allocate the string from the user's memory pool */
  expanded_template = apr_palloc( user_mp, length + 1 );
  apr_brigade_flatten( template->bb, expanded_template, &flatten_len );
  expanded_template[length] = '\0';

  return expanded_template;
}
