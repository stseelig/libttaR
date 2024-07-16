//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// alloc.c                                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include "../bits.h"	// HOT

#include "debug.h"

//////////////////////////////////////////////////////////////////////////////

/**@fn malloc_check
 * @brief malloc with a null check
 *
 * @param size allocation size
 *
 * @return the allocated pointer
**/
/*@only@*/ /*@out@*/
HOT void *
malloc_check(const size_t size)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	const void *const restrict r = malloc(size);
	if UNLIKELY ( r == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(r != NULL);
	return (void *) r;
}

/**@fn calloc_check
 * @brief calloc with a null check
 *
 * @param nmemb number of members
 * @param size member size
 *
 * @return the allocated pointer
**/
/*@only@*/ /*@in@*/
HOT void *
calloc_check(const size_t nmemb, const size_t size)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	const void *const restrict r = calloc(nmemb, size);
	if UNLIKELY ( r == NULL ){
		error_sys(errno, "calloc", NULL);
	}
	assert(r != NULL);
	return (void *) r;
}

/**@fn realloc_check
 * @brief realloc with a null check
 *
 * @param ptr the pointer to reallocate
 * @param size reallocation size
 *
 * @return the reallocated pointer
**/
/*@only@*/ /*@partial@*/
HOT void *
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
{
	const void *const r = realloc(ptr, size);
	if UNLIKELY ( r == NULL ){
		error_sys(errno, "realloc", NULL);
	}
	assert(r != NULL);
	return (void *) r;
}

// EOF ///////////////////////////////////////////////////////////////////////
