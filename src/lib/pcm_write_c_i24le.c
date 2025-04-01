//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_write_c_i24le.c                                                //
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

#undef dest
ALWAYS_INLINE void write_i32h_to_i24le(/*@out@*/ u8 *restrict dest, i32)
/*@modifies	*dest@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn pcm_write_i24le
 * @brief writes a buffer of i32 into a buffer of i24le PCM
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
HIDDEN size_t
pcm_write_i24le(
	/*@out@*/ u8 *const restrict dest, const i32 *const restrict src,
	const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += 3u ){
		write_i32h_to_i24le(&dest[j], src[i]);
	}
	/*@-mustdefine@*/
	return i;
	/*@=mustdefine@*/
}

//--------------------------------------------------------------------------//

/**@fn write_i32h_to_i24le
 * @brief writes a sample of i24le PCM from an i32 sample
 *
 * @param dest[out] destination sample
 * @param src[in] source buffer
**/
ALWAYS_INLINE void
write_i32h_to_i24le(/*@out@*/ u8 *const restrict dest, const i32 x)
/*@modifies	*dest@*/
{
	dest[0u] = (u8)  ((u32) x);
	dest[1u] = (u8) (((u32) x) >>  8u);
	dest[2u] = (u8) (((u32) x) >> 16u);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
