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

static int perl_variable_can_be_json( SV *input )
{
  return ( SvOK( input ) && SvROK( input ) &&
           ( ( SvTYPE( SvRV( input ) ) == SVt_PVHV ) ||
             ( SvTYPE( SvRV( input ) ) == SVt_PVAV ) ) );
}


json_t *perl_variable_to_json( apr_pool_t *mp, SV *input )
{
  json_writer_t *writer;

  if ( perl_variable_can_be_json( input ) ) {
    writer = json_writer_create( mp );
    perl_variable_to_json_internal( input, writer );
    return writer->json;
  }
  else {
    return NULL;
  }
}

