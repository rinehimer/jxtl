/*
 * json_writer_ctx.c
 *
 * Description
 *   Implementation of the JSON writer context.
 *
 * Copyright 2010 Dan Rinehimer
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "apr_macros.h"
#include "json_writer_ctx.h"

json_writer_ctx_t *json_writer_ctx_create( apr_pool_t *mp )
{
  json_writer_ctx_t *context;
  context = apr_palloc( mp, sizeof(json_writer_ctx_t) );
  context->mp = mp;
  context->depth = 0;
  context->prop_stack = apr_array_make( context->mp, 1024,
                                        sizeof(unsigned char *) );
  context->state_stack = apr_array_make( context->mp, 1024,
                                         sizeof(json_writer_ctx_state) );
  APR_ARRAY_PUSH( context->state_stack, json_writer_ctx_state ) = JSON_INITIAL;
  return context;
}

json_writer_ctx_state json_writer_ctx_get_state( json_writer_ctx_t *context )
{
  return APR_ARRAY_TAIL( context->state_stack, json_writer_ctx_state );
}

int json_writer_ctx_can_start_object_or_array( json_writer_ctx_t *context )
{
  json_writer_ctx_state state = json_writer_ctx_get_state( context );
  return ( state == JSON_INITIAL || state == JSON_PROPERTY ||
           state == JSON_IN_ARRAY );
}

unsigned char *json_writer_ctx_get_prop( json_writer_ctx_t *context )
{
  return APR_ARRAY_TAIL( context->prop_stack, unsigned char * );
}

int json_writer_ctx_start_object( json_writer_ctx_t *context )
{
  if ( !json_writer_ctx_can_start_object_or_array( context ) )
    return FALSE;

  APR_ARRAY_PUSH( context->state_stack,
                  json_writer_ctx_state ) = JSON_IN_OBJECT;
  return TRUE;
}

int json_writer_ctx_end_object( json_writer_ctx_t *context )
{
  if ( json_writer_ctx_get_state( context ) != JSON_IN_OBJECT )
    return FALSE;

  apr_array_pop( context->state_stack );
  return TRUE;
}

int json_writer_ctx_start_array( json_writer_ctx_t *context )
{
  if ( !json_writer_ctx_can_start_object_or_array( context ) )
    return FALSE;

  APR_ARRAY_PUSH( context->state_stack,
                  json_writer_ctx_state ) = JSON_IN_ARRAY;
  return TRUE;
}

int json_writer_ctx_end_array( json_writer_ctx_t *context )
{
  if ( json_writer_ctx_get_state( context ) != JSON_IN_ARRAY )
    return FALSE;

  apr_array_pop( context->state_stack );
  return TRUE;
}

int json_writer_ctx_start_property( json_writer_ctx_t *context,
                                    unsigned char *name )
{
  unsigned char *name_copy;
  if ( json_writer_ctx_get_state( context ) != JSON_IN_OBJECT )
    return FALSE;

  name_copy = (unsigned char *) apr_pstrdup( context->mp, (char *) name );

  APR_ARRAY_PUSH( context->prop_stack, unsigned char * ) = name_copy;
  APR_ARRAY_PUSH( context->state_stack,
                  json_writer_ctx_state ) = JSON_PROPERTY;
  return TRUE;
}

int json_writer_ctx_end_property( json_writer_ctx_t *context )
{
  apr_array_pop( context->prop_stack );
  apr_array_pop( context->state_stack );
  return TRUE;
}

int json_writer_ctx_can_write_value( json_writer_ctx_t *context )
{
  json_writer_ctx_state state = json_writer_ctx_get_state( context );
  return ( state == JSON_PROPERTY || state == JSON_IN_ARRAY );
}
