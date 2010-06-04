#ifndef LEX_EXTRA_H
#define LEX_EXTRA_H

#include <apr_file_io.h>
#include <apr_pools.h>
#include <apr_tables.h>

/*
** When creating a scanner, this is the object that gets stored in it's
** "extra" slot.  It handles 
*/
typedef struct lex_extra_t {
  apr_status_t status; /* Status variable used when reading from in_file */
  apr_pool_t *mp; /* Memory pool for opening file and allocating str_array */
  apr_array_header_t *str_array;
  apr_file_t *in_file;
  apr_size_t bytes;
}lex_extra_t;

void lex_extra_init( lex_extra_t *lex_extra, const char *filename );
void lex_extra_destroy( lex_extra_t *lex_extra );

#endif
