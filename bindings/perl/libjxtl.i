%module "LibJXTL"

%{
#include "jxtl.h"
#include "json.h"
#include "json_parser.h"
%}

%init %{
  apr_app_initialize( NULL, NULL, NULL );
%}

%typemap(in, numinputs = 0) apr_array_header_t ** (apr_array_header_t *tmp) {
 $1 = &tmp;
}

%typemap(argout) apr_array_header_t ** {
  if ( argvi >= items ) {
    EXTEND( sp, 1 );
  }
  $result = SWIG_NewPointerObj( *($1), SWIGTYPE_p_apr_array_header_t, 0 );
  argvi++;
}

%typemap(in, numinputs = 0) json_t ** (json_t *tmp) {
 $1 = &tmp;
}

%typemap(argout) json_t ** {
  if ( argvi >= items ) {
    EXTEND( sp, 1 );
  }
  $result = SWIG_NewPointerObj( *($1), SWIGTYPE_p_json_t, 0 );
  argvi++;
}

%inline %{
  apr_pool_t *jxtl_pool_create( void ) {
    apr_pool_t *mp;
    apr_pool_create( &mp, NULL );
    return mp;
  }
%}

%include "jxtl.h"
%include "json.h"
%include "json_parser.h"
