/*
 * $Id$
 *
 * Description
 *   Contains some APR macros that may not exist in older versions.  Some are
 *   taken directly from the APR documentation.
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

#ifndef APR_MACROS_H
#define APR_MACROS_H

#ifndef APR_ARRAY_IDX
#define APR_ARRAY_IDX( ary, i, type ) (((type *)(ary)->elts)[i])
#endif

/**
 * The first element of an array.
 */
#define APR_ARRAY_HEAD( ary, type ) APR_ARRAY_IDX( ary, 0, type )

/**
 * The last element of an array.
 */
#define APR_ARRAY_TAIL( ary, type ) APR_ARRAY_IDX( ary, ary->nelts - 1, type )

#ifndef APR_ARRAY_PUSH
#define APR_ARRAY_PUSH( ary, type ) (*((type *)apr_array_push(ary)))
#endif

#ifndef APR_ARRAY_POP
#define APR_ARRAY_POP( ary, type ) (*((type *)apr_array_pop(ary)))
#endif

/**
 * apr_array_clear() function was added in 1.3.  This is all the function does.
 */
#define APR_ARRAY_CLEAR( ary ) ary->nelts = 0;

#endif
