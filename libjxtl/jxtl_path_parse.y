/*
 * $Id$
 *
 * Description
 *   Bison source file for generating the jxtl_path grammar.
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

%{
#include <apr_strings.h>
#include <stdarg.h>

/*
 * Define YY_DECL before including jxtl_path_lex.h so that it knows we are
 * doing a custom declaration of jxtl_path_lex.
 */
#define YY_DECL

#include "apr_macros.h"
#include "parser.h"
#include "jxtl_path_parse.h"
#include "jxtl_path_lex.h"
#include "jxtl_path.h"

#define callbacks ((jxtl_path_callback_t *) callbacks_ptr)

void jxtl_path_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
                      void *callbacks_ptr, const char *error_string, ... );
%}

%name-prefix="jxtl_path_"
%defines
%verbose
%locations
%error-verbose

%pure-parser

%parse-param { yyscan_t scanner }
%parse-param { parser_t *parser }
%parse-param { void *callbacks_ptr }
%lex-param { yyscan_t scanner }

%union {
  int ival;
  unsigned char *string;
}

%token T_IDENTIFIER "identifier" T_PARENT ".."

%left '/'
%nonassoc '!'

%%

path_expr
  : '!' pattern { callbacks->negate_handler( callbacks->user_data ); }
  | pattern
;

pattern
  : '/' { callbacks->root_object_handler( callbacks->user_data ); } path_pattern
  | path_pattern
;

path_pattern
  : T_IDENTIFIER { callbacks->identifier_handler( callbacks->user_data,
                                                  $<string>1 ); } predicate
  | '.' { callbacks->current_object_handler( callbacks->user_data ); }
  | T_PARENT { callbacks->parent_object_handler( callbacks->user_data ); }
  | '*' { callbacks->any_object_handler( callbacks->user_data ); } predicate
  | path_pattern '/' path_pattern
;

predicate
  : /* empty */
  | '[' { callbacks->predicate_start_handler( callbacks->user_data ); }
     path_expr ']'
     { callbacks->predicate_end_handler( callbacks->user_data ); }
;

%%

/**
 * Parser error callback.  Just store the error so that the caller can retrieve
 * it later and show it in a context that makes sense.
 */
void jxtl_path_error( YYLTYPE *yylloc, yyscan_t scanner, parser_t *parser,
		      void *callbacks_ptr, const char *error_string, ... )
{
  va_list args;
  va_start( args, error_string );
  parser->error_str = apr_pvsprintf( parser->mp, error_string, args );
  va_end( args );
}

typedef struct path_data_t {
  apr_pool_t *mp;
  /** Array to store the current expression. */
  apr_array_header_t *expr_array;
  /** Root of the path expression. */
  jxtl_path_expr_t *root;
  /** The current expression. */
  jxtl_path_expr_t *curr;
}path_data_t;

/**
 * Convenience function called by the parser callbacks.
 */
static void create_expr( path_data_t *data,
                         jxtl_path_expr_type type,
                         unsigned char *identifier )
{
  jxtl_path_expr_t *expr;

  expr = apr_palloc( data->mp, sizeof( jxtl_path_expr_t ) );
  expr->type = type;
  expr->identifier = identifier;
  expr->root = ( data->root ) ? data->root : expr;
  expr->next = NULL;
  expr->predicate = NULL;
  expr->negate = 0;

  if ( !data->root ) {
    data->root = expr;
  }

  if ( !data->curr ) {
    data->curr = expr;
  }
  else {
    data->curr->next = expr;
    data->curr = expr;
  }
}

/*
 * Begin parser callback functions.
 */

/**
 * Request to lookup an identifier.
 */
static void lookup_identifier( void *user_data, unsigned char *ident )
{
  path_data_t *data = (path_data_t *) user_data;
  create_expr( data, JXTL_PATH_LOOKUP,
               (unsigned char *) apr_pstrdup( data->mp, (char *) ident ) );
}

/**
 * Request for the root object.
 */
static void get_root_object( void *user_data )
{
  create_expr( user_data, JXTL_PATH_ROOT_OBJ, NULL );
}

/**
 * Request for the parent object.
 */
static void get_parent_object( void *user_data )
{
  create_expr( user_data, JXTL_PATH_PARENT_OBJ, NULL );
}

/**
 * Request for the current object.
 */
static void get_current_object( void *user_data )
{
  create_expr( user_data, JXTL_PATH_CURRENT_OBJ, NULL );
}

/**
 * Request for any object.
 */
static void get_any_object( void *user_data )
{
  create_expr( user_data, JXTL_PATH_ANY_OBJ, NULL );
}

/**
 * Start a predicate.  Push the current expression node on our stack and set
 * root and curr to NULL.
 */
static void start_predicate( void *user_data )
{
  path_data_t *data = (path_data_t *) user_data;

  APR_ARRAY_PUSH( data->expr_array, jxtl_path_expr_t * ) = data->curr;

  data->root = NULL;
  data->curr = NULL;
}

/**
 * End a predicate.  Pop off the previous expression node, set its predicate
 * to be this expression and reset the curr and root pointers from what we
 * popped.
 */
static void end_predicate( void *user_data )
{
  path_data_t *data = (path_data_t *) user_data;
  jxtl_path_expr_t *expr;
  expr = APR_ARRAY_POP( data->expr_array, jxtl_path_expr_t * );

  expr->predicate = data->root;
  data->curr = expr;
  data->root = expr->root;
}

/**
 * Negate the current expression.
 */
static void negate_expression( void *user_data )
{
  path_data_t *data = (path_data_t *) user_data;
  data->root->negate = 1;
}

/*
 * End of parser callback functions.
 */

/**
 * Create a path parser.
 */
parser_t *jxtl_path_parser_create( apr_pool_t *mp )
{
  parser_t *parser = parser_create( mp,
				    jxtl_path_lex_init,
				    jxtl_path_set_extra,
				    jxtl_path_lex_destroy,
				    jxtl_path__scan_buffer,
				    jxtl_path__delete_buffer,
				    jxtl_path_parse );
  jxtl_path_callback_t *jxtl_callbacks;
  path_data_t *jxtl_data;

  jxtl_callbacks = apr_palloc( mp, sizeof(jxtl_path_callback_t) );
  
  jxtl_callbacks->identifier_handler = lookup_identifier;
  jxtl_callbacks->root_object_handler = get_root_object;
  jxtl_callbacks->parent_object_handler = get_parent_object;
  jxtl_callbacks->current_object_handler = get_current_object;
  jxtl_callbacks->any_object_handler = get_any_object;
  jxtl_callbacks->predicate_start_handler = start_predicate;
  jxtl_callbacks->predicate_end_handler = end_predicate;
  jxtl_callbacks->negate_handler = negate_expression;

  jxtl_data = apr_palloc( mp, sizeof(path_data_t) );
  jxtl_data->expr_array = apr_array_make( mp, 32, sizeof(jxtl_path_expr_t *) );
  jxtl_data->mp = mp;
  jxtl_data->root = NULL;
  jxtl_data->curr = NULL;

  jxtl_callbacks->user_data = jxtl_data;

  parser_set_user_data( parser, jxtl_callbacks );

  return parser;
}

int jxtl_path_parser_parse_buffer( parser_t *parser,
                                   const char *path,
                                   jxtl_path_expr_t **expr )
{
  jxtl_path_callback_t *jxtl_callbacks = parser_get_user_data( parser );
  path_data_t *jxtl_data = jxtl_callbacks->user_data;

  APR_ARRAY_CLEAR( jxtl_data->expr_array );
  jxtl_data->root = NULL;
  jxtl_data->curr = NULL;
  *expr = NULL;

  if ( parser_parse_buffer( parser, path ) == 0 ) {
    *expr = jxtl_data->root;
  }

  return parser->parse_result;
}
