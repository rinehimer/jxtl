%module "libjxtl"

%{
#include <apr_general.h>
#include "json.h"
#include "template.h"
#include "template_funcs.h"
#include "py_util.h"
%}

%include "template.i"

/**
 * Convert an XML file to a dictionary type.
 */
PyObject *xml_to_dict( const char *xml_file );

/**
 * Convert a JSON file to a dictionary type.
 */
PyObject *json_to_dict( const char *json_file );
