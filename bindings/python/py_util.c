#include <Python.h>
#include <pyport.h>
#include "json_writer.h"

#ifndef Py_ssize_t
#define Py_ssize_t int
#endif

static void py_dict_to_json( PyObject *obj, json_writer_t *writer );
static void py_list_to_json( PyObject *obj, json_writer_t *writer );

static void py_variable_to_json_internal( PyObject *obj,
                                          json_writer_t *writer )
{
  if ( PyString_Check( obj ) ) {
    json_writer_write_string( writer,
                              (unsigned char *) PyString_AsString( obj ) );
  }
  else if ( PyDict_Check( obj ) ) {
    py_dict_to_json( obj, writer );
  }
  else if ( PyList_Check( obj ) ) {
    py_list_to_json( obj, writer );
  }
}

static void py_dict_to_json( PyObject *obj, json_writer_t *writer )
{
  PyObject *key, *value;
  Py_ssize_t pos = 0;

  json_writer_start_object( writer );

  while ( PyDict_Next( obj, &pos, &key, &value ) ) {
    json_writer_start_property( writer,
                                (unsigned char *) PyString_AsString( key ) );
    py_variable_to_json_internal( value, writer );
    json_writer_end_property( writer );
  }

  json_writer_end_object( writer );
}

static void py_list_to_json( PyObject *obj, json_writer_t *writer )
{
  Py_ssize_t i;
  Py_ssize_t n = PyList_GET_SIZE( obj );
  PyObject *tmp;

  json_writer_start_array( writer );

  for ( i = 0; i < n; i++ ) {
    tmp = PyList_GET_ITEM( obj, i );
    py_variable_to_json_internal( tmp, writer );
  }

  json_writer_end_array( writer );
}


json_t *py_variable_to_json( apr_pool_t *mp, PyObject *obj )
{
  json_writer_t *writer;
  json_t *json = NULL;
  apr_pool_t *tmp_mp;

  if ( PyDict_Check( obj ) || PyList_Check( obj ) ) {
    apr_pool_create( &tmp_mp, NULL );
    writer = json_writer_create( tmp_mp, mp );
    py_variable_to_json_internal( obj, writer );
    json = writer->json;
    apr_pool_destroy( tmp_mp );
  }
  else {
    fprintf( stderr, "Must be an object or a list\n" );
  }

  return json;

}
