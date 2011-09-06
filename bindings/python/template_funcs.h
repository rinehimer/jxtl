#ifndef TEMPLATE_FUNCS_H
#define TEMPLATE_FUNCS_H

#include <Python.h>

#include "template.h"
#include "json.h"

char *python_format_func( json_t *json, char *format,
                          void *template_ptr );
#endif
