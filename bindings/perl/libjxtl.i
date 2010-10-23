%module "LibJXTL"

%{
#include <apr_general.h>
#include "json.h"
#include "template.h"
#include "template_funcs.h"
#include "perl_util.h"
%}

%include "template.i"

/**
 * Convert an XML file to a hash type.
 */
SV *xml_to_hash( char *xml_file );
