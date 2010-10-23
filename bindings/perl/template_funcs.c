#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include <apr_pools.h>
#include <apr_strings.h>

#include "json.h"
#include "jxtl.h"

#include "template.h"
#include "perl_util.h"

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

void Template_set_format_callback( Template *t, SV *format_func )
{
  if ( SvROK( format_func ) &&
       SvTYPE( SvRV( format_func ) ) == SVt_PVCV ) {
    if ( t->format_func ) {
      SvREFCNT_dec( t->format_func );
    }
    t->format_func = SvRV( format_func );
    SvREFCNT_inc( t->format_func );
  }
  else {
    fprintf( stderr,
             "Error setting format function: not a code reference\n" );
  }
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
