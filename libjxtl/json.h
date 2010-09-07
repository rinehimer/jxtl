#ifndef JSON_H
#define JSON_H

#include <apr_general.h>
#include <apr_hash.h>
#include <apr_pools.h>
#include <apr_tables.h>

/**
 * JSON types
 */
typedef enum json_type {
  JSON_STRING,
  JSON_INTEGER,
  JSON_NUMBER,
  JSON_OBJECT,
  JSON_ARRAY,
  JSON_BOOLEAN,
  JSON_NULL
} json_type;

typedef struct json_t {
  unsigned char *name;
  json_type type;
  struct json_t *parent;
  union {
    unsigned char *string;
    int integer;
    double number;
    apr_hash_t *object;
    apr_array_header_t *array;
    int boolean;
  } value;
} json_t;

#define JSON_NAME( json ) (json)->name
#define JSON_IS_TYPE( json, json_type ) ( (json)->type == json_type )

#define JSON_IS_STRING( json ) JSON_IS_TYPE( json, JSON_STRING )
#define JSON_IS_INTEGER( json ) JSON_IS_TYPE( json, JSON_INTEGER )
#define JSON_IS_NUMBER( json ) JSON_IS_TYPE( json, JSON_NUMBER )
#define JSON_IS_OBJECT( json ) JSON_IS_TYPE( json, JSON_OBJECT )
#define JSON_IS_ARRAY( json ) JSON_IS_TYPE( json, JSON_ARRAY )
#define JSON_IS_BOOLEAN( json ) JSON_IS_TYPE( json, JSON_BOOLEAN )
#define JSON_IS_NULL( json ) JSON_IS_NULL( json, JSON_NULL )

#define JSON_IS_TRUE_BOOLEAN( json )\
 ( JSON_IS_BOOLEAN( json ) && (json)->value.boolean == TRUE )

json_t *json_string_create( apr_pool_t *mp, unsigned char *string );
json_t *json_integer_create( apr_pool_t *mp, int integer );
json_t *json_number_create( apr_pool_t *mp, double number );
json_t *json_object_create( apr_pool_t *mp );
json_t *json_array_create( apr_pool_t *mp );
json_t *json_boolean_create( apr_pool_t *mp, int boolean );
json_t *json_null_create( apr_pool_t *mp );

void json_object_print( json_t *node, int indent );
void json_to_xml( json_t *json, int indent );

#endif
