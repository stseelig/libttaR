//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_write_a_u8.c                                                   //
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

ALWAYS_INLINE CONST u8 i32h_to_u8(i32) /*@*/;

//////////////////////////////////////////////////////////////////////////////

/**@fn pcm_write_u8
 * @brief writes a buffer of i32 into a buffer of u8 PCM
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
HIDDEN size_t
pcm_write_u8(
	/*@out@*/ u8 *const restrict dest, const i32 *const restrict src,
	const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i;

	assert(nsamples != 0);

	for ( i = 0; i < nsamples; ++i ){
		dest[i] = i32h_to_u8(src[i]);
	}
	/*@-mustdefine@*/
	return i;
	/*@=mustdefine@*/
}

//--------------------------------------------------------------------------//

/**@fn i32h_to_u8
 * @brief converts an i32 sample from a u8 PCM sample
 *
 * @param x u8 sample
 *
 * @return i32 sample
**/
ALWAYS_INLINE CONST u8
i32h_to_u8(const i32 x)
/*@*/
{
	return (u8) (x + 0x80u);
}

// EOF ///////////////////////////////////////////////////////////////////////
