#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include <apr_pools.h>
#include <apr_strings.h>

#include "json.h"
#include "jxtl.h"
#include "xml2json.h"

#include "template.h"
#include "perl_util.h"

Template *new_Template( char *buffer )
{
  Template *t = malloc( sizeof(Template) );
  apr_pool_create( &t->mp, NULL );

  t->jxtl_parser = jxtl_parser_create( t->mp );
  t->template = NULL;
  t->json = NULL;
  t->format_func = NULL;

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

void Template_set_context( Template *t, json_t *json )
{
  t->json = json;
}

static char *format_func( json_t *json, char *format, void *template_ptr )
{
  Template *t = (Template *) template_ptr;
  char *value = json_get_string_value( t->mp, json );
  int n;
  SV *context = sv_newmortal();
  SV *perl_ret;
  char *ret_val = NULL;

  if ( !value )
    value = "";

  dSP;
  ENTER;
  SAVETMPS;
  PUSHMARK( SP );

  sv_setref_pv( context, "_p_json_t", (json_t *) json );

  XPUSHs( sv_2mortal( newSVpv( value, 0 ) ) );
  XPUSHs( sv_2mortal( newSVpv( format, 0 ) ) );
  XPUSHs( context );
  PUTBACK;

  n = call_sv( t->format_func, G_SCALAR );

  SPAGAIN;
  if ( n == 1 ) {
    perl_ret = POPs;
    ret_val = apr_pstrdup( t->mp, SvPV_nolen( perl_ret ) );
  }

  PUTBACK;
  FREETMPS;
  LEAVE;

  return ret_val;
}

void Template_set_format_callback( Template *t, SV *perl_format_func )
{
  if ( SvROK( perl_format_func ) &&
       SvTYPE( SvRV( perl_format_func ) ) == SVt_PVCV ) {
    t->format_func = SvRV( perl_format_func );
  }
  else {
    fprintf( stderr,
             "Error setting format function: not a code reference\n" );
  }
}

SV *Template_xml_to_hash( Template *t, char *xml_file )
{
  apr_pool_t *tmp_mp;
  json_t *json;
  SV *hash = &PL_sv_undef;

  apr_pool_create( &tmp_mp, NULL );
  xml_file_to_json( tmp_mp, xml_file, 1, &json );
  if ( json ) {
    hash = json_to_perl_variable( json );
  }
  apr_pool_destroy( tmp_mp );

  return hash;
} 

int Template_expand_to_file( Template *t, char *file, SV *input )
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
    t->json = perl_variable_to_json( tmp_mp, input );
  }

  status = ( jxtl_expand_to_file( t->template, t->json, file ) == 0 );

  apr_pool_destroy( tmp_mp );

  return status;
}

char *Template_expand_to_buffer( Template *t, SV *input )
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
    t->json = perl_variable_to_json( tmp_mp, input );
  }

  buffer = jxtl_expand_to_buffer( t->mp, t->template, t->json );
  apr_pool_destroy( tmp_mp );

  return buffer;
}
