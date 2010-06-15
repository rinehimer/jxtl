#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_strings.h>
#include <apr_tables.h>

#include "json_node.h"

#define JSON_CREATE( mp, json )					   \
  json = apr_palloc( mp, sizeof( json_t ) );                       \
  json->name = NULL;						   \
  json->parent = NULL

json_t *json_string_create( apr_pool_t *mp, unsigned char *string )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.string = apr_pstrdup( mp, string );
  json->type = JSON_STRING;
  return json;
}

json_t *json_integer_create( apr_pool_t *mp, int integer )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.integer = integer;
  json->type = JSON_INTEGER;
  return json;
}

json_t *json_number_create( apr_pool_t *mp, double number )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.number = number;
  json->type = JSON_NUMBER;
  return json;
}

json_t *json_object_create( apr_pool_t *mp )
{
  json_t *json;

  JSON_CREATE( mp, json );
  json->value.object = apr_hash_make( mp );
  json->type = JSON_OBJECT;
  return json;
}

json_t *json_array_create( apr_pool_t *mp )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.array = apr_array_make( mp, 8, sizeof( json_t * ) );
  json->type = JSON_ARRAY;  
  return json;
}

json_t *json_boolean_create( apr_pool_t *mp, int boolean )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->value.boolean = boolean;
  json->type = JSON_BOOLEAN;
  return json;
}

json_t *json_null_create( apr_pool_t *mp )
{
  json_t *json;
  JSON_CREATE( mp, json );
  json->type = JSON_NULL;
  return json;
}
