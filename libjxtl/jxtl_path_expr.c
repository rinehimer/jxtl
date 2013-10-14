/*
 * jxtl_path_expr.c
 *
 * Description
 *   Contains the implementation of the jxtl path expr object.
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

#include <stdlib.h>

#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "jxtl_path.h"
#include "jxtl_path_expr.h"
#include "parser.h"

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
                         char *identifier )
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
static void identifier_handler( void *user_data, char *ident )
{
  path_data_t *data = (path_data_t *) user_data;
  create_expr( data, JXTL_PATH_LOOKUP, apr_pstrdup( data->mp, ident ) );
}

/**
 * Create an expression to lookup a variable.
 */
static void variable_handler( void *user_data, char *ident )
{
  path_data_t *data = (path_data_t *) user_data;
  create_expr( data, JXTL_PATH_VARIABLE, apr_pstrdup( data->mp, ident ) );
}

/**
 * Request for the root object.
 */
static void root_object_handler( void *user_data )
{
  create_expr( user_data, JXTL_PATH_ROOT_OBJ, NULL );
}

/**
 * Request for the parent object.
 */
static void parent_object_handler( void *user_data )
{
  create_expr( user_data, JXTL_PATH_PARENT_OBJ, NULL );
}

/**
 * Request for the current object.
 */
static void current_object_handler( void *user_data )
{
  create_expr( user_data, JXTL_PATH_CURRENT_OBJ, NULL );
}

/**
 * Request for any object.
 */
static void any_object_handler( void *user_data )
{
  create_expr( user_data, JXTL_PATH_ANY_OBJ, NULL );
}

/**
 * Start a predicate.  Push the current expression node on our stack and set
 * root and curr to NULL.
 */
static void predicate_start_handler( void *user_data )
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
static void predicate_end_handler( void *user_data )
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
static void negate_handler( void *user_data )
{
  path_data_t *data = (path_data_t *) user_data;
  data->root->negate = 1;
}

static void initialize_callbacks( apr_pool_t *mp,
				  apr_pool_t *tmp_mp,
				  jxtl_path_callback_t *callbacks,
				  path_data_t *cb_data )
{
  callbacks->identifier_handler = identifier_handler;
  callbacks->variable_handler = variable_handler;
  callbacks->root_object_handler = root_object_handler;
  callbacks->parent_object_handler = parent_object_handler;
  callbacks->current_object_handler = current_object_handler;
  callbacks->any_object_handler = any_object_handler;
  callbacks->predicate_start_handler = predicate_start_handler;
  callbacks->predicate_end_handler = predicate_end_handler;
  callbacks->negate_handler = negate_handler;

  cb_data->expr_array = apr_array_make( tmp_mp, 32,
					sizeof(jxtl_path_expr_t *) );
  cb_data->mp = mp;
  cb_data->root = NULL;
  cb_data->curr = NULL;

  callbacks->user_data = cb_data;
}

int jxtl_path_parser_parse_buffer_to_expr( apr_pool_t *mp,
					   parser_t *parser,
                                           const char *buf,
                                           jxtl_path_expr_t **expr )
{
  int result = FALSE;
  apr_pool_t *tmp_mp;
  jxtl_path_callback_t callbacks;
  path_data_t cb_data;

  apr_pool_create( &tmp_mp, NULL );

  initialize_callbacks( mp, tmp_mp, &callbacks, &cb_data );

  *expr = NULL;

  if ( jxtl_path_parser_parse_buffer( parser, buf, &callbacks ) ) {
    *expr = cb_data.root;
    result = TRUE;
  }

  apr_pool_destroy( tmp_mp );  
  return result;
}
