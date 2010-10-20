%{
#include <apr_general.h>
#include <apr_pools.h>
#include "json.h"
#include "jxtl.h"
#include "template.h"
%}

%init %{
  apr_app_initialize( NULL, NULL, NULL );
%}

#if defined(SWIGPERL)
# define FORMAT_FUNC_T SV *
# define DICTIONARY_T SV *
#elif defined(SWIGPYTHON)
# define FORMAT_FUNC_T PyObject *
# define DICTIONARY_T PyObject *
#endif

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
  Template( char *buffer = NULL )
  {
    Template *t = malloc( sizeof(Template) );
    apr_pool_create( &t->mp, NULL );
    
    t->jxtl_parser = jxtl_parser_create( t->mp );
    t->template = NULL;
    t->json = NULL;
    t->format_func = NULL;
    
    if ( buffer ) {
      jxtl_parser_parse_buffer( t->jxtl_parser, buffer, &t->template );
    }
    
    return t;
  }
  
  /**
   * Destructor.
   */
  ~Template()
  {
    apr_pool_destroy( self->mp );
    free( self );
  }
  
  /**
   * Load a template from a file.
   */
  int load( char *file )
  {
    return ( jxtl_parser_parse_file( self->jxtl_parser, file,
                                     &self->template ) == 0 );
  }
  
  /**
   * Set the context for a template.
   */
  void set_context( json_t *json )
  {
    self->json = json;
  }

  /**
   * Set the format callback for the template.
   */
  void set_format_callback( FORMAT_FUNC_T format_func );

  /**
   * Expand a template to a file using a Perl hash reference or the existing
   * context of the template.
   */
  int expand_to_file( char *file, DICTIONARY_T input = NULL );
  
  /**
   * Expand a template to a buffer using a Perl hash reference or the
   * existing context of the template.
   */
  char *expand_to_buffer( DICTIONARY_T input = NULL );
}

/**
 * Convert an XML file to a dictionary/hash type.
 */
DICTIONARY_T xml_to_hash( char *xml_file );
