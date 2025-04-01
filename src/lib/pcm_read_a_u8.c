//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_read_a_u8.c                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE CONST i32 u8_to_i32h(u8) /*@*/;

//////////////////////////////////////////////////////////////////////////////

/**@fn pcm_read_u8
 * @brief reads a buffer of u8 PCM into a buffer of i32
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
HIDDEN size_t
pcm_read_u8(
	/*@out@*/ i32 *const restrict dest, const u8 *const restrict src,
	const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i;
	for ( i = 0; i < nsamples; ++i ){
		dest[i] = u8_to_i32h(src[i]);
	}
	/*@-mustdefine@*/
	return i;
	/*@=mustdefine@*/
}

//--------------------------------------------------------------------------//

/**@fn u8_to_i32h
 * @brief converts a u8 PCM sample into an i32 sample
 *
 * @param x u8 sample
 *
 * @return i32 sample
**/
ALWAYS_INLINE CONST i32
u8_to_i32h(const u8 x)
/*@*/
{
	return (i32) (x - 0x80u);
}

// EOF ///////////////////////////////////////////////////////////////////////
