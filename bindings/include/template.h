#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <apr_hash.h>
#include <apr_pools.h>
#include "jxtl.h"
#include "json.h"
#include "parser.h"

/**
 * Define the base Template type.  This type is extended in template.i by using
 * SWIG to add functions.
 */
typedef struct Template {
  apr_pool_t *mp;
  parser_t *jxtl_parser;
  jxtl_template_t *template;
  json_t *json;
  apr_hash_t *formats;
}Template;

void register_format_funcs( Template *t, jxtl_format_func format_func );

#endif
