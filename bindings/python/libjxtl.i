%module "libjxtl"

%{
#include <apr_general.h>
#include "json.h"
#include "template.h"
#include "template_funcs.h"
%}

%include "template.i"
