%module "LibJXTL"

%{
#include "perl_util.h"
#include <apr_general.h>
%}

%init %{
  apr_app_initialize( NULL, NULL, NULL );
%}

int expand_to_file( const char *template_file, SV *input,
                    const char *output_file );
SV *expand_to_buffer( const char *template_file, SV *input );
