#ifndef XML2JSON_H
#define XML2JSON_H

#include <apr_pools.h>
#include "json.h"

int xml_to_json( apr_pool_t *mp, apr_file_t *xml_file, int skip_root,
                 json_t **json );

#endif
