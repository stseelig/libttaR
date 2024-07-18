#ifndef TTA_ALLOC_H
#define TTA_ALLOC_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// alloc.h                                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "../bits.h"	// HOT

//////////////////////////////////////////////////////////////////////////////

/*@only@*/ /*@out@*/
HOT void *malloc_check(size_t size)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

/*@only@*/ /*@in@*/
HOT void *calloc_check(size_t nmemb, size_t size)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

/*@only@*/ /*@partial@*/
HOT void *realloc_check(
	/*@only@*/ /*@null@*/ /*@out@*/ void *ptr, size_t size
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*ptr
@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
