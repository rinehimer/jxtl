#ifndef PY_UTIL_H
#define PY_UTIL_H

#include <Python.h>
#include <apr_pools.h>
#include "json.h"

/*
 * Macros to support Python 3.  Some of these are defined by Swig, so we use
 * ifndef to avoid redeclaration warnings of those.
 */
#if PY_MAJOR_VERSION >= 3
#  define PyString_CheckExact PyUnicode_CheckExact
#  ifndef PyString_AS_STRING
#    define PyString_AS_STRING(o) PyUnicode_AsUTF8(o)
#  endif
#  ifndef PyString_AsString
#    define PyString_AsString(o) PyUnicode_AsUTF8(o)
#  endif
#  ifndef PyString_FromString
#    define PyString_FromString(o) PyUnicode_FromString(o)
#  endif
#  ifndef PyInt_FromLong
#   define PyInt_FromLong(i) PyLong_FromLong(i)
#  endif
#  define PyInt_CheckExact(o) PyLong_CheckExact(o)
#  define PyInt_AS_LONG(o) PyLong_AsLong(o)
#endif

json_t *py_variable_to_json( apr_pool_t *mp, PyObject *obj );
PyObject *xml_to_dict( const char *xml_file );
PyObject *json_to_dict( const char *json_file );
PyObject *verify_python_function( PyObject *func );

#endif
