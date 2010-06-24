#include "json_path_parse.h"
#include "json_path_lex.h"
#include "lex_extra.h"

int json_path_file_parse( const char *file )
{
  lex_extra_t lex_extra;
  yyscan_t json_path_scanner;
  int parse_result;

  json_lex_init( &json_path_scanner );
  lex_extra_init( &lex_extra, file );
  json_set_extra( &lex_extra, json_path_scanner );
  parse_result = json_path_parse( json_path_scanner, NULL );
  lex_extra_destroy( &lex_extra );
  json_lex_destroy( json_path_scanner );

  return parse_result;
}
