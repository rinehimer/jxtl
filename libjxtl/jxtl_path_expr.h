/*
 * $Id$
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

#ifndef JXTL_PATH_EXPR_H
#define JXTL_PATH_EXPR_H

#include <apr_pools.h>

#include "parser.h"

typedef enum jxtl_path_expr_type {
  JXTL_PATH_ROOT_OBJ,
  JXTL_PATH_PARENT_OBJ,
  JXTL_PATH_CURRENT_OBJ,
  JXTL_PATH_ANY_OBJ,
  JXTL_PATH_LOOKUP,
} jxtl_path_expr_type;

typedef struct jxtl_path_expr_t {
  /** What type of expression this is. */
  jxtl_path_expr_type type;
  /** A name to lookup. */
  unsigned char *identifier;
  /** The beginning of this expression. */
  struct jxtl_path_expr_t *root;
  /** Next expression. */
  struct jxtl_path_expr_t *next;
  /** A predicate to evaluate. */
  struct jxtl_path_expr_t *predicate;
  /** Whether or not this expression should be negated. */
  int negate;
} jxtl_path_expr_t;

int jxtl_path_parser_parse_buffer_to_expr( apr_pool_t *mp, parser_t *parser,
                                           const unsigned char *buf,
                                           jxtl_path_expr_t **expr );

#endif
