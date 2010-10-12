#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include "json.h"
#include "jxtl.h"

#include "template.h"
#include "perl_util.h"

Template *new_Template( char *buffer )
{
  Template *t = malloc( sizeof(Template) );
  apr_pool_create( &t->mp, NULL );

  t->jxtl_parser = jxtl_parser_create( t->mp );
  t->template = NULL;
  t->json = NULL;

  if ( buffer ) {
    jxtl_parser_parse_buffer( t->jxtl_parser, buffer, &t->template );
  }

  return t;
}

void delete_Template( Template *t )
{
  apr_pool_destroy( t->mp );
  free( t );
}

int Template_load( Template *t, char *file )
{
  return ( jxtl_parser_parse_file( t->jxtl_parser, file, &t->template ) == 0 );
}

void Templeate_set_context( Template *t, json_t *json )
{
  t->json = json;
}

int Template_expand_to_file( Template *t, char *file, SV *input,
                             SV *perl_format_func )
{
  apr_pool_t *tmp_mp;
  int status;

  apr_pool_create( &tmp_mp, NULL );

  if ( input ) {
    t->json = perl_variable_to_json( tmp_mp, input );
  }

  status = ( jxtl_expand_to_file( t->template, t->json, file ) == 0 );

  apr_pool_destroy( tmp_mp );

  return status;
}

char *Template_expand_to_buffer( Template *t, SV *input, SV *perl_format_func )
{
  char *buffer;
  apr_pool_t *tmp_mp;

  apr_pool_create( &tmp_mp, NULL );

  if ( input ) {
    t->json = perl_variable_to_json( tmp_mp, input );
  }

  buffer = jxtl_expand_to_buffer( t->mp, t->template, t->json );
  apr_pool_destroy( tmp_mp );

  return buffer;
}
