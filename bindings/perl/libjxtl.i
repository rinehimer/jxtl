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
 * Convert an XML file to a hash.
 */
SV *xml_to_hash( const char *xml_file );

/**
 * Convert a JSON file to a hash.
 */
SV *json_to_hash( const char *json_file );
