/*
 * This file contains some APR macros that may not exist in older versions.
 * Some are taken directly from the APR documentation.
 */

#ifndef APR_MACROS_H
#define APR_MACROS_H

#ifndef APR_ARRAY_IDX
#define APR_ARRAY_IDX( ary, i, type ) (((type *)(ary)->elts)[i])
#endif

#ifndef APR_ARRAY_PUSH
#define APR_ARRAY_PUSH( ary, type ) (*((type *)apr_array_push(ary)))
#endif

#ifndef APR_ARRAY_POP
#define APR_ARRAY_POP( ary, type ) (*((type *)apr_array_pop(ary)))
#endif

/*
 * apr_array_clear() function was added in 1.3.  Not sure why it doesn't exist
 * as a macro.
 */
#define APR_ARRAY_CLEAR( ary ) ary->nelts = 0;

#endif
