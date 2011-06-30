#include <Python.h>

#include <apr_pools.h>
#include <apr_strings.h>

#include "json.h"
#include "jxtl.h"
#include "py_util.h"
#include "template.h"

static char *python_format_func( json_t *json, char *format,
                                 void *template_ptr )
{
  Template *t = (Template *) template_ptr;
  char *value = json_get_string_value( t->mp, json );
  char *ret_val = NULL;
  PyObject *format_func;
  PyObject *arglist;
  PyObject *py_ret;
  PyObject *py_json;

  format_func = apr_hash_get( t->formats, format, APR_HASH_KEY_STRING );

  py_json = PyCObject_FromVoidPtrAndDesc( json, "_p_json_t", NULL );
  arglist = Py_BuildValue( "ssO", value, format, py_json );
  py_ret = PyObject_CallObject( format_func, arglist );

  if ( PyString_CheckExact( py_ret ) ) {
    ret_val = apr_pstrdup( t->mp, PyString_AS_STRING( py_ret ) );
  }

  return ret_val;
}

void Template_register_format( Template *t, const char *format,
                               PyObject *format_func )
{
  if ( PyCallable_Check( format_func ) ) {
    /**TODO: Check whether we should increment ref counts. */
    apr_hash_set( t->formats, format, APR_HASH_KEY_STRING, format_func );
  }
  else {
    fprintf( stderr, "Error setting format function: not callable\n" );
  }
}

int Template_expand_to_file( Template *t, char *file, PyObject *input )
{
  apr_pool_t *tmp_mp;
  int status;

  if ( ! t->template ) {
    fprintf( stderr, "Error: a template must be loaded before expanding.\n" );
    return FALSE;
  }

  apr_pool_create( &tmp_mp, NULL );
  register_format_funcs( t, python_format_func );

  if ( input ) {
    t->json = py_variable_to_json( t->mp, input );
  }

  status = ( jxtl_expand_to_file( t->template, t->json, file ) == 0 );

  apr_pool_destroy( tmp_mp );

  return status;
}

char *Template_expand_to_buffer( Template *t, PyObject *input )
{
  char *buffer;
  apr_pool_t *tmp_mp;

  if ( ! t->template ) {
    fprintf( stderr, "Error: a template must be loaded before expanding.\n" );
    return "";
  }

  apr_pool_create( &tmp_mp, NULL );
  register_format_funcs( t, python_format_func );

  if ( input ) {
    t->json = py_variable_to_json( t->mp, input );
  }

  buffer = jxtl_expand_to_buffer( t->mp, t->template, t->json );
  apr_pool_destroy( tmp_mp );

  return buffer;
}
