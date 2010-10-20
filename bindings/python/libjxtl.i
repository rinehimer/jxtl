%module "libjxtl"

%{
#include <apr_general.h>
#include "json.h"
#include "template.h"
#include "template_funcs.h"
#include "py_util.h"
%}

%include "template.i"
