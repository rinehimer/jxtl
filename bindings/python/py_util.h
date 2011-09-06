#ifndef PY_UTIL_H
#define PY_UTIL_H

#include <Python.h>
#include <apr_pools.h>
#include "json.h"

json_t *py_variable_to_json( apr_pool_t *mp, PyObject *obj );
PyObject *xml_to_dict( const char *xml_file );
PyObject *json_to_dict( const char *json_file );
PyObject *verify_python_function( PyObject *func );

#endif
