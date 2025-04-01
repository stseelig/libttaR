//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_read_0.c                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// only u8, i16le, and i24le supported                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"

//////////////////////////////////////////////////////////////////////////////

#undef dest
extern HIDDEN size_t pcm_read_u8(
	/*@out@*/ i32 *restrict dest, const u8 *restrict, size_t
)
/*@modifies	*dest@*/
;

#undef dest
extern HIDDEN size_t pcm_read_i16le(
	/*@out@*/ i32 *restrict dest, const u8 *restrict, size_t
)
/*@modifies	*dest@*/
;

#undef dest
extern HIDDEN size_t pcm_read_i24le(
	/*@out@*/ i32 *restrict dest, const u8 *restrict, size_t
)
/*@modifies	*dest@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn libttaR_pcm_read
 * @brief reads a buffer of PCM into a buffer of i32
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 * @param samplebytes number of bytes per PCM sample
 *
 * @return 'nsamples' on success
 * @retval 0 error
 *
 * @note read the manpage for more info
**/
/*@unused@*/
size_t
libttaR_pcm_read(
	/*@out@*/ i32 *const restrict dest, const u8 *const restrict src,
	const size_t nsamples, const enum LibTTAr_SampleBytes samplebytes
)
/*@modifies	*dest@*/
{
	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
		return pcm_read_u8(dest, src, nsamples);
	case LIBTTAr_SAMPLEBYTES_2:
		return pcm_read_i16le(dest, src, nsamples);
	case LIBTTAr_SAMPLEBYTES_3:
		return pcm_read_i24le(dest, src, nsamples);
	}
	/*@notreached@*/
	return 0;
}

// EOF ///////////////////////////////////////////////////////////////////////
