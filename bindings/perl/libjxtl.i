%module "LibJXTL"

%{
#include <apr_general.h>
#include <apr_pools.h>
#include "perl_util.h"
#include "json.h"
#include "jxtl.h"
%}

%init %{
  apr_app_initialize( NULL, NULL, NULL );
%}

%include "template.h"

 /**
  * Using the magic of SWIG we can create a class for templates.
  */
%extend Template {
  /**
   * Constructor - create a new template from a buffer or just allocate the
   * object if no buffer is passed. Calls new_Template().
   */
  Template( char *buffer = NULL );
  
  /**
   * Destructor.  Calls delete_Template().
   */
  ~Template();
  
  /**
   * Load a template from a file.  Calls Template_load().
   */
  int load( char *file );
  
  /**
   * Set the context for a template.  Calls Template_set_context().
   */
  void set_context( json_t *json );

  /**
   * Set the format function to call.  Calls Template_set_format_func().
   */
  void set_format_func( SV *perl_format_func );
  
  /**
   * Expand a template to a file using a Perl hash reference or the existing
   * context of the template.
   */
  int expand_to_file( char *file, SV *input = NULL,
                      SV *perl_format_func = NULL );
  
  /**
   * Expand a template to a buffer using a Perl hash reference or the
   * existing context of the template.
   */
  char *expand_to_buffer( SV *input = NULL, SV *perl_format_func = NULL );
}
