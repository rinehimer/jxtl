#include <stdlib.h>

#include "json_path.h"
#include "json_path_parse.h"
#include "json_path_lex.h"
#include "lex_extra.h"

int json_path_compile( const char *path )
{
  yyscan_t scanner;
  lex_extra_t lex_extra;
  YY_BUFFER_STATE buffer_state;
  int parse_result;
  char *eval_str;
  int eval_str_len;

  /* Set up eval_str for flex.  Flex requires the last two bytes of a string
  ** passed to yy_scan_buffer be the null terminator.
  */
  eval_str_len = strlen( path ) + 2;
  eval_str = malloc( eval_str_len );
  apr_cpystrn( eval_str, path, eval_str_len - 1 );
  eval_str[eval_str_len - 1] = '\0';

  json_path_lex_init( &scanner );
  buffer_state = json_path__scan_buffer( eval_str, eval_str_len, scanner );
  lex_extra_init( &lex_extra, NULL );
  json_path_set_extra( &lex_extra, scanner );
  parse_result = json_path_parse( scanner );
  lex_extra_destroy( &lex_extra );
  json_path_lex_destroy( scanner );
  
  free( eval_str );

  return 1;
}
