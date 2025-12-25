/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// alloc.c                                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "./common.h"
#include "./debug.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn malloc_check
 * @brief malloc with a null check
 *
 * @param size - allocation size
 *
 * @return the allocated pointer
**/

HOT
/*@only@*/ /*@out@*/
BUILD NOINLINE void *
malloc_check(const size_t size)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
/*@-mustmod@*/
{
	void *const RESTRICT retval = malloc(size);

	if UNLIKELY ( retval == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(retval != NULL);

	return retval;
}
/*@=mustmod@*/

/**@fn calloc_check
 * @brief calloc with a null check
 *
 * @param nmemb - number of members
 * @param size  - member size
 *
 * @return the allocated pointer
**/
HOT
/*@only@*/ /*@in@*/
BUILD NOINLINE void *
calloc_check(const size_t nmemb, const size_t size)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
/*@-mustmod@*/
{
	void *const RESTRICT retval = calloc(nmemb, size);

	if UNLIKELY ( retval == NULL ){
		error_sys(errno, "calloc", NULL);
	}
	assert(retval != NULL);

	return retval;
}
/*@=mustmod@*/

/**@fn realloc_check
 * @brief realloc with a null check
 *
 * @param ptr  - pointer to reallocate
 * @param size - reallocation size
 *
 * @return the reallocated pointer
**/
HOT
/*@only@*/ /*@partial@*/
BUILD NOINLINE void *
realloc_check(
	/*@only@*/ /*@null@*/ /*@out@*/ void *const ptr, const size_t size
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*ptr
@*/
/*@-mustmod@*/
{
	void *const retval = realloc(ptr, size);

	if UNLIKELY ( retval == NULL ){
		error_sys(errno, "realloc", NULL);
	}
	assert(retval != NULL);

	return retval;
}
/*@=mustmod@*/

/* EOF //////////////////////////////////////////////////////////////////// */
