#include <Python.h>

#include <apr_pools.h>
#include <apr_strings.h>

#include "json.h"
#include "jxtl.h"
#include "py_util.h"
#include "template.h"

static char *format_func( json_t *json, char *format, void *template_ptr )
{
  Template *t = (Template *) template_ptr;
  char *value = json_get_string_value( t->mp, json );

  return value;
}

void Template_set_format_callback( Template *t, PyObject *format_func )
{
  t->format_func = format_func;
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

  if ( t->format_func ) {
    jxtl_template_set_format_func( t->template, format_func );
    jxtl_template_set_format_data( t->template, t );
  }

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

  if ( t->format_func ) {
    jxtl_template_set_format_func( t->template, format_func );
    jxtl_template_set_format_data( t->template, t );
  }

  if ( input ) {
    t->json = py_variable_to_json( t->mp, input );
  }

  buffer = jxtl_expand_to_buffer( t->mp, t->template, t->json );
  apr_pool_destroy( tmp_mp );

  return buffer;
}
