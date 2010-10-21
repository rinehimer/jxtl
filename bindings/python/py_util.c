#include <Python.h>
#include <pyport.h>
#include "json.h"
#include "json_writer.h"

/* Not sure what the best way to go about this is... */
#if !defined(HAVE_SSIZE_T) || SIZEOF_VOID_P != SIZEOF_SIZE_T
#define Py_ssize_t int
#endif

static void py_dict_to_json( PyObject *obj, json_writer_t *writer );
static void py_list_to_json( PyObject *obj, json_writer_t *writer );
static void py_tuple_to_json( PyObject *obj, json_writer_t *writer );

static void py_variable_to_json_internal( PyObject *obj,
                                          json_writer_t *writer )
{
  if ( PyString_CheckExact( obj ) ) {
    json_writer_write_string( writer,
                              (unsigned char *) PyString_AS_STRING( obj ) );
  }
  else if ( PyInt_CheckExact( obj ) ) {
    json_writer_write_integer( writer, PyInt_AS_LONG( obj ) );
  }
  else if ( PyFloat_CheckExact( obj ) ) {
    json_writer_write_number( writer, PyFloat_AS_DOUBLE( obj ) );
  }
  else if ( PyBool_Check( obj ) ) {
    json_writer_write_boolean( writer, ( obj == Py_True ) );
  }
  else if ( PyUnicode_CheckExact( obj ) ) {
    /* Create a new string object that is UTF-8 encoded. */
    Py_UNICODE *unicode = PyUnicode_AS_UNICODE( obj );
    Py_ssize_t size = PyUnicode_GET_SIZE( obj );
    PyObject *str_obj = PyUnicode_EncodeUTF8( unicode, size, NULL );
    py_variable_to_json_internal( str_obj, writer );
    PyObject_Free( str_obj );
  }
  else if ( PyDict_CheckExact( obj ) ) {
    py_dict_to_json( obj, writer );
  }
  else if ( PyList_CheckExact( obj ) ) {
    py_list_to_json( obj, writer );
  }
  else if ( PyTuple_CheckExact( obj ) ) {
    py_tuple_to_json( obj, writer );
    
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
  Py_ssize_t size = PyList_GET_SIZE( obj );
  PyObject *tmp;

  json_writer_start_array( writer );

  for ( i = 0; i < size; i++ ) {
    tmp = PyList_GET_ITEM( obj, i );
    py_variable_to_json_internal( tmp, writer );
  }

  json_writer_end_array( writer );
}

static void py_tuple_to_json( PyObject *obj, json_writer_t *writer )
{
  Py_ssize_t i;
  Py_ssize_t size = PyTuple_GET_SIZE( obj );
  PyObject *tmp;

  /*
   * There's not a good way to represent a tuple in JSON, we just make it an
   * array.
   */
  json_writer_start_array( writer );

  for ( i = 0 ; i < size; i++ ) {
    tmp = PyTuple_GET_ITEM( obj, i );
    py_variable_to_json_internal( tmp, writer );
  }

  json_writer_end_array( writer );
}


json_t *py_variable_to_json( apr_pool_t *mp, PyObject *obj )
{
  json_writer_t *writer;
  json_t *json = NULL;
  apr_pool_t *tmp_mp;

  if ( PyDict_CheckExact( obj ) || PyList_CheckExact( obj ) ||
       PyTuple_CheckExact( obj ) ) {
    apr_pool_create( &tmp_mp, NULL );
    writer = json_writer_create( tmp_mp, mp );
    py_variable_to_json_internal( obj, writer );
    json = writer->json;
    apr_pool_destroy( tmp_mp );
    json_dump( json, 1);
  }
  else {
    fprintf( stderr, "Must be an object, list or tuple.\n" );
  }

  return json;

}
