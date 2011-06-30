#ifndef TEMPLATE_FUNCS_H
#define TEMPLATE_FUNCS_H

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "template.h"
#include "json.h"

void Template_register_format( Template *t, const char* foramt,
                               SV *format_func );
int Template_expand_to_file( Template *t, char *file, SV *input );
char *Template_expand_to_buffer( Template *t, SV *input );

#endif
