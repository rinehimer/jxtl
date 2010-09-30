#ifndef PERL_UTIL_H
#define PERL_UTIL_H

#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

int expand_to_file( const char *template_file, SV *input,
                    const char *output_file, SV *perl_format_func );
SV *expand_to_buffer( const char *template_file, SV *input,
                      SV *perl_format_func );

#endif
