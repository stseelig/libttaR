//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_write.c                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
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
static NOINLINE size_t pcm_write_u8(/*@out@*/ u8 *dest, const i32 *, size_t)
/*@modifies	*dest@*/
;

#undef dest
static NOINLINE size_t pcm_write_i16le(/*@out@*/ u8 *dest, const i32 *, size_t)
/*@modifies	*dest@*/
;

#undef dest
static NOINLINE size_t pcm_write_i24le(/*@out@*/ u8 *dest, const i32 *, size_t)
/*@modifies	*dest@*/
;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST u8 i32h_to_u8(i32) /*@*/;

#undef dest
ALWAYS_INLINE void write_i32h_to_i16le(/*@out@*/ u8 *restrict dest, i32)
/*@modifies	*dest@*/
;

#undef dest
ALWAYS_INLINE void write_i32h_to_i24le(/*@out@*/ u8 *restrict dest, i32)
/*@modifies	*dest@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn libttaR_pcm_write
 * @brief reads a buffer of i32 into a buffer of PCM
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
size_t
libttaR_pcm_write(
	/*@out@*/ u8 *const dest, const i32 *const src, const size_t nsamples,
	const enum TTASampleBytes samplebytes
)
/*@modifies	*dest@*/
{
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
		return pcm_write_u8(dest, src, nsamples);
	case TTASAMPLEBYTES_2:
		return pcm_write_i16le(dest, src, nsamples);
	case TTASAMPLEBYTES_3:
		return pcm_write_i24le(dest, src, nsamples);
	}
	/*@notreached@*/
	return 0;
}

//--------------------------------------------------------------------------//

/**@fn pcm_write_u8
 * @brief writes a buffer of i32 into a buffer of u8 PCM
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
static NOINLINE size_t
pcm_write_u8(
	/*@out@*/ u8 *const dest, const i32 *const src, const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i;
	for ( i = 0; i < nsamples; ++i ){
		dest[i] = i32h_to_u8(src[i]);
	}
	return i;
}

/**@fn pcm_write_i16le
 * @brief writes a buffer of i32 into a buffer of i16le PCM
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
static NOINLINE size_t
pcm_write_i16le(
	/*@out@*/ u8 *const dest, const i32 *const src, const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += 2u ){
		write_i32h_to_i16le(&dest[j], src[i]);
	}
	return i;
}

/**@fn pcm_write_i24le
 * @brief writes a buffer of i32 into a buffer of i24le PCM
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
static NOINLINE size_t
pcm_write_i24le(
	/*@out@*/ u8 *const dest, const i32 *const src, const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += 3u ){
		write_i32h_to_i24le(&dest[j], src[i]);
	}
	return i;
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

/**@fn write_i32h_to_i16le
 * @brief writes a sample of i16le PCM from an i32 sample
 *
 * @param dest[out] destination sample
 * @param src[in] source buffer
**/
ALWAYS_INLINE void
write_i32h_to_i16le(/*@out@*/ u8 *const restrict dest, const i32 x)
/*@modifies	*dest@*/
{
	dest[0u] = (u8)  ((u32) x);
	dest[1u] = (u8) (((u32) x) >> 8u);
	return;
}

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
