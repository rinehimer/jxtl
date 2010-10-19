#ifndef TEMPLATE_H
#define TEMPLATE_H

#include <apr_pools.h>
#include "jxtl.h"
#include "json.h"

typedef struct Template {
  apr_pool_t *mp;
  parser_t *jxtl_parser;
  jxtl_template_t *template;
  json_t *json;
  void *format_func;
}Template;

#endif
