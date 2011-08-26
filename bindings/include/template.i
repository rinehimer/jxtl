%{
#include <apr_general.h>
#include <apr_hash.h>
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
# define FORMAT_FUNC perl_format_func
# define TO_JSON_FUNC perl_variable_to_json
#elif defined(SWIGPYTHON)
# define FORMAT_FUNC_T PyObject *
# define DICTIONARY_T PyObject *
# define FORMAT_FUNC python_format_func
# define TO_JSON_FUNC py_variable_to_json
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
    return jxtl_parser_parse_file( self->jxtl_parser, file, &self->template );
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
  void register_format( const char *format, FORMAT_FUNC_T format_func );

  /**
   * Expand a template to a file using a language specific dictionary or the
   * existing context of the template.
   */
  int expand_to_file( char *file, DICTIONARY_T input = NULL )
  {
    apr_pool_t *tmp_mp;
    int status;

    if ( !self->template ) {
      fprintf( stderr,
               "Error: a template must be loaded before expanding.\n" );
      return FALSE;
    }

    apr_pool_create( &tmp_mp, NULL );
    register_format_funcs( self, FORMAT_FUNC );
    
    if ( input ) {
      self->json = TO_JSON_FUNC( self->mp, input );
    }

    status = ( jxtl_expand_to_file( self->template, self->json, file ) == 0 );

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
      fprintf( stderr,
               "Error: a template must be loaded before expanding.\n" );
      return "";
    }

    apr_pool_create( &tmp_mp, NULL );
    register_format_funcs( self, FORMAT_FUNC );

    if ( input ) {
      self->json = TO_JSON_FUNC( tmp_mp, input );
    }

    buffer = jxtl_expand_to_buffer( self->mp, self->template, self->json );
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
