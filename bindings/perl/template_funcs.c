#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include <apr_pools.h>
#include <apr_strings.h>

#include "json.h"
#include "jxtl.h"

#include "template.h"
#include "perl_util.h"

static char *perl_format_func( json_t *json, char *format, void *template_ptr )
{
  Template *t = (Template *) template_ptr;
  char *value = json_get_string_value( t->mp, json );
  int n;
  SV *format_func;
  SV *context = sv_newmortal();
  SV *perl_ret;
  char *ret_val = NULL;

  if ( !value )
    value = "";

  format_func = apr_hash_get( t->formats, format, APR_HASH_KEY_STRING );

  dSP;
  ENTER;
  SAVETMPS;
  PUSHMARK( SP );

  sv_setref_pv( context, "_p_json_t", (json_t *) json );

  XPUSHs( sv_2mortal( newSVpv( value, 0 ) ) );
  XPUSHs( sv_2mortal( newSVpv( format, 0 ) ) );
  XPUSHs( context );
  PUTBACK;

  n = call_sv( format_func, G_SCALAR );

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

void Template_register_format( Template *t, const char *format,
                               SV *format_func )
{
  SV *func_ptr;

  if ( SvROK( format_func ) &&
       SvTYPE( SvRV( format_func ) ) == SVt_PVCV ) {
    /* TODO:  Check whether or not we should mess with refcount. */
    func_ptr = SvRV( format_func );
      /* SvREFCNT_inc( func_ptr ); */
    apr_hash_set( t->formats, format, APR_HASH_KEY_STRING, func_ptr );
  }
  else {
    fprintf( stderr,
             "Error setting %s format function: not a code reference\n",
             format );
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
  register_format_funcs( t, perl_format_func );

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
  register_format_funcs( t, perl_format_func );

  if ( input ) {
    t->json = perl_variable_to_json( tmp_mp, input );
  }

  buffer = jxtl_expand_to_buffer( t->mp, t->template, t->json );
  apr_pool_destroy( tmp_mp );

  return buffer;
}
