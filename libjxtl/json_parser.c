#include <apr_pools.h>

#include "json_parse.h"
#include "json_lex.h"
#include "json_parser.h"
#include "json_writer.h"
#include "lex_extra.h"


int json_file_parse( const char *json_file, json_writer_t *writer )
{
  lex_extra_t lex_extra;
  yyscan_t json_scanner;
  apr_pool_t *json_mp;
  int parse_result;
  json_callback_t callbacks = {
    json_writer_object_start,
    json_writer_object_end,
    json_writer_array_start,
    json_writer_array_end,
    json_writer_property_start,
    json_writer_property_end,
    json_writer_string_write,
    json_writer_integer_write,
    json_writer_number_write,
    json_writer_boolean_write,
    json_writer_null_write,
    writer
  };

  apr_pool_create( &json_mp, NULL );
  json_lex_init( &json_scanner );
  lex_extra_init( &lex_extra, json_file );
  json_set_extra( &lex_extra, json_scanner );
  parse_result = json_parse( json_scanner, &callbacks );
  lex_extra_destroy( &lex_extra );
  json_lex_destroy( json_scanner );

  return parse_result;
}
