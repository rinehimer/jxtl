#ifndef TEMPLATE_FUNCS_H
#define TEMPLATE_FUNCS_H

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "template.h"
#include "json.h"

void Template_register_format( Template *t, const char* foramt,
                               SV *format_func );
char *perl_format_func( json_t *json, char *format, void *template_ptr );

#endif
