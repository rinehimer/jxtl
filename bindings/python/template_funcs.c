#include <Python.h>

#include <apr_pools.h>
#include <apr_strings.h>

#include "json.h"
#include "jxtl.h"
#include "py_util.h"
#include "template.h"
#include "capsulethunk.h"

char *python_format_func( json_t *json, char *format, void *template_ptr )
{
  Template *t = (Template *) template_ptr;
  char *value = json_get_string_value( t->mp, json );
  char *ret_val = NULL;
  PyObject *format_func;
  PyObject *arglist;
  PyObject *py_ret;
  PyObject *py_json;

  format_func = apr_hash_get( t->formats, format, APR_HASH_KEY_STRING );

  py_json = PyCapsule_New( json, "_p_json_t", NULL );
  PyCapsule_SetContext( py_json, "_p_json_t" );
  arglist = Py_BuildValue( "ssO", value, format, py_json );
  py_ret = PyObject_CallObject( format_func, arglist );

  if ( PyString_CheckExact( py_ret ) ) {
    ret_val = apr_pstrdup( t->mp, PyString_AS_STRING( py_ret ) );
  }

  return ret_val;
}
