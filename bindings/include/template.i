%{
#include <apr_general.h>
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include "json.h"
#include "jxtl.h"
#include "jxtl_template.h"
#include "misc.h"
#include "template.h"
%}

%init %{
  apr_app_initialize( NULL, NULL, NULL );
%}

#if defined(SWIGPERL)
# define FORMAT_FUNC_T SV *
# define DICTIONARY_T SV *
# define FORMAT_FUNC perl_format_func
# define TO_JSON_FUNC perl_variable_to_json
# define VERIFY_FUNC verify_perl_function
#elif defined(SWIGPYTHON)
# define FORMAT_FUNC_T PyObject *
# define DICTIONARY_T PyObject *
# define FORMAT_FUNC python_format_func
# define TO_JSON_FUNC py_variable_to_json
# define VERIFY_FUNC verify_python_function
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
    t->formats = apr_hash_make( t->mp );
    
    if ( buffer ) {
      jxtl_parser_parse_buffer_to_template( t->mp, t->jxtl_parser, buffer,
					    &t->template );
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
  int load( const char *file )
  {
    apr_file_t *input_file;
    return ( open_apr_input_file( self->mp, file, &input_file ) &&
             jxtl_parser_parse_file_to_template( self->mp, self->jxtl_parser,
                                                 input_file, &self->template ) );
  }
  
  /**
   * Set the context for a template.  This would be used from a format callback
   * to do inside expansion.
   */
  void set_context( json_t *json )
  {
    self->json = json;
  }

  /**
   * Register a format callback for the template.
   */
  void register_format( const char *format, FORMAT_FUNC_T format_func )
  {
    char error_buf[200];
    FORMAT_FUNC_T func_ptr = VERIFY_FUNC( format_func );

    if ( func_ptr ) {
      apr_hash_set( self->formats, apr_pstrdup( self->mp, format ), APR_HASH_KEY_STRING, func_ptr );
    }
    else {
      snprintf( error_buf, sizeof(error_buf),
                "Error registering format \"%s\".  "
                "The function reference is not valid.", format );
      SWIG_Error( SWIG_TypeError, error_buf );
    }
  }

  /**
   * Expand a template to a file using a language specific dictionary or the
   * existing context of the template.
   */
  int expand_to_file( const char *file, DICTIONARY_T input = NULL )
  {
    apr_pool_t *tmp_mp;
    apr_file_t *out;
    int status;

    if ( !self->template ) {
      SWIG_Error( SWIG_RuntimeError,
                  "A template must be loaded before expanding." );
      return FALSE;
    }
    
    apr_pool_create( &tmp_mp, NULL );
    register_format_funcs( self, FORMAT_FUNC );
    
    if ( input ) {
      self->json = TO_JSON_FUNC( self->mp, input );
    }

    if ( ! open_apr_output_file( tmp_mp, file, &out ) ) {
      SWIG_Error( SWIG_RuntimeError,
                  "Failed to open output file." );
      return FALSE;
    }

    status = ( jxtl_template_expand_to_file( self->template, self->json,
					     out ) == 0 );

    apr_pool_destroy( tmp_mp );

    return status;
  }
  
  /**
   * Expand a template to a buffer using a language specific dictionary or the
   * existing context of the template.
   */
  char *expand_to_buffer( DICTIONARY_T input = NULL )
  {
    char *buffer;
    apr_pool_t *tmp_mp;
    
    if ( !self->template ) {
      SWIG_Error( SWIG_RuntimeError,
                  "A template must be loaded before expanding." );
      return "";
    }

    apr_pool_create( &tmp_mp, NULL );
    register_format_funcs( self, FORMAT_FUNC );

    if ( input ) {
      self->json = TO_JSON_FUNC( tmp_mp, input );
    }

    buffer = jxtl_template_expand_to_buffer( self->mp, self->template,
					     self->json );
    apr_pool_destroy( tmp_mp );

    return buffer;
  }
}

%{
  /*
   * Utility function called from individual template implementations to
   * actually register the format functions.
   */
  void register_format_funcs( Template *t, jxtl_format_func format_func )
  {
    apr_hash_index_t *idx;
    const char *format;

    if ( apr_hash_count( t->formats ) > 0 ) {
      /**
       * Register the same callback for all functions in here.  We'll call the
       * correct one from our callback.
       */
      for ( idx = apr_hash_first( NULL, t->formats ); idx;
            idx = apr_hash_next( idx ) ) {
        apr_hash_this( idx, (const void **) &format, NULL, NULL );
        jxtl_template_register_format( t->template, format, format_func );
      }
      jxtl_template_set_format_data( t->template, t );
    }
  }
%}
