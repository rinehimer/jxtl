#ifndef XML2JSON_H
#define XML2JSON_H

#include <apr_pools.h>

int xml_file_to_json( apr_pool_t *mp, const char *filename, int skip_root,
                      json_t **json  );

#endif
