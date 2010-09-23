#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include <apr_pools.h>

#include "perl_util.h"
#include "json_writer.h"
#include "json.h"

void perl_hash_to_json( SV *input, json_writer_t *writer );
void perl_array_to_json( SV *input, json_writer_t *writer );

static void perl_variable_to_json_internal( SV *input, json_writer_t *writer )
{
  char *val;
  int type = -1;
  
  if ( SvOK( input ) && SvROK( input ) ) {
    type = SvTYPE( SvRV( input ) );
  }
  else if ( SvOK( input ) ) {
    type = SvTYPE( input );
  }

  switch ( type ) {
  case SVt_IV:
  case SVt_NV:
  case SVt_PV:
  case SVt_RV:
    val = SvPV_nolen( input );
    json_writer_string_write( writer, (unsigned char *) val );
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

  json_writer_object_start( writer );

  if ( ( SvROK( input ) && SvTYPE( SvRV( input ) ) == SVt_PVHV ) ) {
    h = (HV *) SvRV( input );
    cnt = hv_iterinit( h );
    while ( cnt-- ) {
      item = hv_iternextsv( h, &prop, &retlen );
      json_writer_property_start( writer, (unsigned char *) prop );
      perl_variable_to_json_internal( item, writer );
      json_writer_property_end( writer );
    }
  }

  json_writer_object_end( writer );
}

static void perl_array_to_json( SV *input, json_writer_t *writer )
{
  int len;
  AV *array;
  SV **item;

  json_writer_array_start( writer );

  if ( SvROK( input ) && SvTYPE( SvRV( input ) ) == SVt_PVAV ) {
    array = (AV *) SvRV( input );
    len = av_len( array ) + 1;

    while ( len-- ) {
      item = av_fetch( array, len, 0 );
      perl_variable_to_json_internal( *item, writer );
    }
  }

  json_writer_array_end( writer );
}



json_t *perl_variable_to_json( SV *input )
{
  apr_pool_t *mp;
  json_writer_t *writer;
  apr_pool_create( &mp, NULL );

  writer = json_writer_create( mp );
  perl_variable_to_json_internal( input, writer );

  json_object_print( writer->json, 1 );

  return writer->json;
}

static int perl_variable_can_be_json( SV *input )
{
  return ( SvOK( input ) && SvROK( input ) &&
           ( SvTYPE( SvRV( input ) ) == SVt_PVHV ) ||
           ( SvTYPE( SvRV( input ) ) == SVt_PVAV ) );
}
