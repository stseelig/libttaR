#ifndef H_TTA_ALLOC_H
#define H_TTA_ALLOC_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// alloc.h                                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stddef.h>

#include "./common.h"

/* //////////////////////////////////////////////////////////////////////// */

HOT
/*@only@*/ /*@out@*/
BUILD_EXTERN NOINLINE void *malloc_check(size_t)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

HOT
/*@only@*/ /*@in@*/
BUILD_EXTERN NOINLINE void *calloc_check(size_t, size_t)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

#undef ptr
HOT
/*@only@*/ /*@partial@*/
BUILD_EXTERN NOINLINE void *realloc_check(
	/*@only@*/ /*@null@*/ /*@out@*/ void *ptr, size_t
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*ptr
@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_ALLOC_H */
