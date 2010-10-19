%module "LibJXTL"

%{
#include <apr_general.h>
#include "json.h"
#include "template.h"
#include "template_funcs.h"
#include "perl_util.h"
%}

%init %{
  apr_app_initialize( NULL, NULL, NULL );
%}

%include "template.h"

 /**
  * Using SWIG we can create a class for the typedef Template.  The functions
  * called are Template_<func_name> for each function defined in here except
  * for the constructor and destructor which call new_Template() and
  * delete_Template(), respectively.
  */
%extend Template {
  /**
   * Constructor - create a new template from a buffer or just allocate the
   * object if no buffer is passed.
   */
  Template( char *buffer = NULL );
  
  /**
   * Destructor.
   */
  ~Template();
  
  /**
   * Load a template from a file.
   */
  int load( char *file );
  
  /**
   * Set the context for a template.
   */
  void set_context( json_t *json );

  /**
   * Set the format callback for the template.
   */
  void set_format_callback( SV *perl_format_func );

  /**
   * Expand a template to a file using a Perl hash reference or the existing
   * context of the template.
   */
  int expand_to_file( char *file, SV *input = NULL );
  
  /**
   * Expand a template to a buffer using a Perl hash reference or the
   * existing context of the template.
   */
  char *expand_to_buffer( SV *input = NULL );
}

/**
 * Convert an XML file to a Perl hash.
 */
SV *xml_to_hash( char *xml_file );
