//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_read_i24le.c                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PURE i32 read_i24le_to_i32h(const u8 *restrict) /*@*/;

//////////////////////////////////////////////////////////////////////////////

/**@fn pcm_read_i24le
 * @brief reads a buffer of i24le PCM into a buffer of i32
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
HIDDEN size_t
pcm_read_i24le(
	/*@out@*/ i32 *const restrict dest, const u8 *const restrict src,
	const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i, j;

	for ( i = 0, j = 0; i < nsamples; ++i, j += 3u ){
		dest[i] = read_i24le_to_i32h(&src[j]);
	}
	/*@-mustdefine@*/
	return i;
	/*@=mustdefine@*/
}

//--------------------------------------------------------------------------//

/**@fn read_i24le_to_i32h
 * @brief reads a sample of i24le PCM into an i32 sample
 *
 * @param src[in] source buffer
 *
 * @return i32 sample
**/
ALWAYS_INLINE PURE i32
read_i24le_to_i32h(const u8 *const restrict src)
/*@*/
{
	u32 retval = 0;
	retval |=  (u32)       src[0u];
	retval |= ((u32)       src[1u])  <<  8u;
	retval |= ((u32) ((i8) src[2u])) << 16u;
	return (i32) retval;
}

// EOF ///////////////////////////////////////////////////////////////////////
