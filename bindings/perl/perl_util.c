#include <EXTERN.h>
#include <perl.h>
#include <XSUB.h>

#include <apr_general.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "perl_util.h"
#include "apr_macros.h"
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
  apr_pool_t *tmp_mp;

  if ( perl_variable_can_be_json( input ) ) {
    apr_pool_create( &tmp_mp, NULL );
    writer = json_writer_create( tmp_mp, mp );
    perl_variable_to_json_internal( input, writer );
    apr_pool_destroy( tmp_mp );
    return writer->json;
  }
  else {
    return NULL;
  }
}

SV *json_to_perl_variable( json_t *json )
{
  json_t *tmp_json;
  apr_array_header_t *arr;
  apr_hash_index_t *idx;
  AV *p_array;
  HV *hash;
  int i;

  switch ( json->type ) {
  case JSON_STRING:
    return newSVpv( (char *) json->value.string, 0 );
    break;

  case JSON_INTEGER:
    return newSViv( json->value.integer );
    break;

  case JSON_NUMBER:
    return newSVnv( json->value.number );
    break;

  case JSON_OBJECT:
    hash = newHV();
    for ( idx = apr_hash_first( NULL, json->value.object ); idx;
          idx = apr_hash_next( idx ) ) {
      apr_hash_this( idx, NULL, NULL, (void **) &tmp_json );
      hv_store( hash, (char *) JSON_NAME( tmp_json ),
                strlen( (char * ) JSON_NAME( tmp_json ) ),
                json_to_perl_variable( tmp_json ), 0 );
    }
    return newRV_noinc( (SV*) hash);
    break;

  case JSON_ARRAY:
    arr = json->value.array;
    p_array = newAV();
    for ( i = 0; arr && i < arr->nelts; i++ ) {
      tmp_json = APR_ARRAY_IDX( arr, i, json_t * );
      av_push( p_array, json_to_perl_variable( tmp_json ) );
    }
    return newRV_noinc( (SV*) p_array);
    break;

  case JSON_BOOLEAN:
    return ( json->value.boolean ) ? newSVpv( "true", 0 ) :
                                     newSVpv( "false", 0 );
    break;

  case JSON_NULL:
    return newSV( 0 );
    break;
    
  default:
    return NULL;
    break;
  }
}
