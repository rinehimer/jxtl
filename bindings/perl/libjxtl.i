%module "LibJXTL"

%{
#include <apr_general.h>
#include "json.h"
#include "template.h"
#include "template_funcs.h"
#include "perl_util.h"
%}

%include "template.i"
