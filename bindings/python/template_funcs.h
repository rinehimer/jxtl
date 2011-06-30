#ifndef TEMPLATE_FUNCS_H
#define TEMPLATE_FUNCS_H

#include <Python.h>

#include "template.h"
#include "json.h"

void Template_register_format( Template *t, const char *format,
                               PyObject *format_func );
int Template_expand_to_file( Template *t, char *file, PyObject *input );
char *Template_expand_to_buffer( Template *t, PyObject *input );

#endif
