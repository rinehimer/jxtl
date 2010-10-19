#ifndef TEMPLATE_FUNCS_H
#define TEMPLATE_FUNCS_H

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "template.h"
#include "json.h"

Template *new_Template( char *buffer );
void delete_Template( Template *t );
int Template_load( Template *t, char *file );
void Template_set_context( Template *t, json_t *json );
void Template_set_format_callback( Template *t, SV *perl_format_func );
int Template_expand_to_file( Template *t, char *file, SV *input );
char *Template_expand_to_buffer( Template *t, SV *input );

#endif
