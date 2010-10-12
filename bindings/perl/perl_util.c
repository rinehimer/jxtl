#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include <apr_general.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "perl_util.h"
#include "parser.h"
#include "json_writer.h"
#include "json.h"
#include "jxtl.h"

static void perl_hash_to_json( SV *input, json_writer_t *writer );
static void perl_array_to_json( SV *input, json_writer_t *writer );

static void perl_variable_to_json_internal( SV *input, json_writer_t *writer )
{
  char *val;
  int ival;
  double number;
  int type = -1;
  
  if ( SvOK( input ) && SvROK( input ) ) {
    type = SvTYPE( SvRV( input ) );
  }
  else if ( SvOK( input ) ) {
    type = SvTYPE( input );
  }

  switch ( type ) {
  case SVt_IV:
    ival = SvIV( input );
    json_writer_write_integer( writer, ival );
    break;

  case SVt_NV:
    number = SvNV( input );
    json_writer_write_number( writer, number );
    break;

  case SVt_PV:
    val = SvPV_nolen( input );
    json_writer_write_string( writer, (unsigned char *) val );
    break;
      
  case SVt_PVAV:
    perl_array_to_json( input, writer );
    break;
      
  case SVt_PVHV:
    perl_hash_to_json( input, writer );
    break;
      
  default:
    break;
  }
}

static void perl_hash_to_json( SV *input, json_writer_t *writer )
{
  HV *h;
  char *prop;
  I32 cnt, retlen;
  SV *item;

  json_writer_start_object( writer );

  if ( ( SvROK( input ) && SvTYPE( SvRV( input ) ) == SVt_PVHV ) ) {
    h = (HV *) SvRV( input );
    cnt = hv_iterinit( h );
    while ( cnt-- ) {
      item = hv_iternextsv( h, &prop, &retlen );
      json_writer_start_property( writer, (unsigned char *) prop );
      perl_variable_to_json_internal( item, writer );
      json_writer_end_property( writer );
    }
  }

  json_writer_end_object( writer );
}

static void perl_array_to_json( SV *input, json_writer_t *writer )
{
  int len;
  AV *array;
  SV **item;
  int i;

  json_writer_start_array( writer );

  if ( SvROK( input ) && SvTYPE( SvRV( input ) ) == SVt_PVAV ) {
    array = (AV *) SvRV( input );
    len = av_len( array ) + 1;
    for ( i = 0; i < len; i++ ) {
      item = av_fetch( array, i, 0 );
      perl_variable_to_json_internal( *item, writer );
    }
  }

  json_writer_end_array( writer );
}

json_t *perl_variable_to_json( apr_pool_t *mp, SV *input )
{
  json_writer_t *writer;

  writer = json_writer_create( mp );
  perl_variable_to_json_internal( input, writer );

  return writer->json;
}

static int perl_variable_can_be_json( SV *input )
{
  return ( SvOK( input ) && SvROK( input ) &&
           ( ( SvTYPE( SvRV( input ) ) == SVt_PVHV ) ||
             ( SvTYPE( SvRV( input ) ) == SVt_PVAV ) ) );
}

typedef struct format_data_t {
  apr_pool_t *mp;
  SV *perl_format_func;
}format_data_t;

static char *format_func( json_t *json, char *format, void *format_data_ptr )
{
  format_data_t *format_data = (format_data_t *) format_data_ptr;
  char *value = json_get_string_value( format_data->mp, json );
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

  sv_setref_pv( context, "void *", (void *) json );

  XPUSHs( sv_2mortal( newSVpv( value, 0 ) ) );
  XPUSHs( sv_2mortal( newSVpv( format, 0 ) ) );
  XPUSHs( context );
  PUTBACK;

  n = call_sv( format_data->perl_format_func, G_SCALAR );

  SPAGAIN;
  if ( n == 1 ) {
    perl_ret = POPs;
    ret_val = apr_pstrdup( format_data->mp, SvPV_nolen( perl_ret ) );
  }

  PUTBACK;
  FREETMPS;
  LEAVE;

  return ret_val;
}

static int prep_for_expand( apr_pool_t **mp_ptr,
                            const char *template_file,
                            jxtl_template_t **template,
                            json_t **json,
                            SV *input,
                            SV *perl_format_func )
{
  parser_t *jxtl_parser;
  int ret = FALSE;
  apr_pool_t *mp;
  format_data_t *format_data;

  apr_pool_create( &mp, NULL );
  jxtl_parser = jxtl_parser_create( mp );

  if ( perl_variable_can_be_json( input ) &&
       jxtl_parser_parse_file( jxtl_parser, template_file,
                               template ) == APR_SUCCESS ) {
    *json = perl_variable_to_json( mp, input );
    format_data = apr_palloc( mp, sizeof(format_data_t) );
    format_data->perl_format_func = perl_format_func;
    format_data->mp = mp;
    jxtl_template_set_format_func( *template, format_func );
    jxtl_template_set_format_data( *template, format_data );
    ret = TRUE;
  }

  *mp_ptr = mp;
  return ret;
}

int expand_to_file( const char *template_file, SV *input,
                    const char *output_file, SV *perl_format_func )
{
  apr_pool_t *mp;
  json_t *json;
  jxtl_template_t *template;
  int ret = FALSE;

  if ( prep_for_expand( &mp, template_file, &template, &json, input,
                        perl_format_func ) ) {
    jxtl_expand_to_file( template, json, output_file );
    ret = TRUE;
  }

  apr_pool_destroy( mp );

  return ret;
}

SV *expand_to_buffer( const char *template_file, SV *input,
                      SV *perl_format_func )
{
  apr_pool_t *mp;
  json_t *json;
  jxtl_template_t *template;
  char *buffer;
  SV *ret = &PL_sv_undef;

  if ( prep_for_expand( &mp, template_file, &template, &json, input,
                        perl_format_func ) ) {
    buffer = jxtl_expand_to_buffer( mp, template, json );
    ret = sv_newmortal();
    sv_setpv( ret, buffer );
  }

  apr_pool_destroy( mp );

  return ret;
}
