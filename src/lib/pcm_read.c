//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_read.c                                                         //
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

#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

#undef dest
NOINLINE size_t pcm_read_u8(i32 *const dest, const u8 *const, size_t)
/*@modifies	*dest@*/
;

#undef dest
NOINLINE size_t pcm_read_i16le(i32 *const dest, const u8 *const, size_t)
/*@modifies	*dest@*/
;

#undef dest
NOINLINE size_t pcm_read_i24le(i32 *const dest, const u8 *const, size_t)
/*@modifies	*dest@*/
;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST i32 u8_to_i32h(register u8) /*@*/;

#undef dest
ALWAYS_INLINE void read_i16le_to_i32h(
	register i32 *const dest, register const u8 *const
)
/*@modifies	*dest@*/
;

#undef dest
ALWAYS_INLINE void read_i24le_to_i32h(
	register i32 *const dest, register const u8 *const
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
size_t
libttaR_pcm_read(
	i32 *const dest, const u8 *const src, size_t nsamples,
	enum TTASampleBytes samplebytes
)
/*@modifies	*dest@*/
{
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
		return pcm_read_u8(dest, src, nsamples);
	case TTASAMPLEBYTES_2:
		return pcm_read_i16le(dest, src, nsamples);
	case TTASAMPLEBYTES_3:
		return pcm_read_i24le(dest, src, nsamples);
	}
	return 0;
}

//--------------------------------------------------------------------------//

/**@fn pcm_read_u8
 * @brief reads a buffer of u8 PCM into a buffer of i32
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
NOINLINE size_t
pcm_read_u8(i32 *const dest, const u8 *const src, size_t nsamples)
/*@modifies	*dest@*/
{
	size_t i;
	for ( i = 0; i < nsamples; ++i ){
		dest[i] = u8_to_i32h(src[i]);
	}
	return i;
}

/**@fn pcm_read_i16le
 * @brief reads a buffer of i16le PCM into a buffer of i32
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
NOINLINE size_t
pcm_read_i16le(i32 *const dest, const u8 *const src, size_t nsamples)
/*@modifies	*dest@*/
{
	size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += (size_t) 2u ){
		read_i16le_to_i32h(&dest[i], &src[j]);
	}
	return i;
}

/**@fn pcm_read_i24le
 * @brief reads a buffer of i24le PCM into a buffer of i32
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param nsamples total number of PCM samples
 *
 * @return 'nsamples'
**/
NOINLINE size_t
pcm_read_i24le(i32 *const dest, const u8 *const src, size_t nsamples)
/*@modifies	*dest@*/
{
	size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += (size_t) 3u ){
		read_i24le_to_i32h(&dest[i], &src[j]);
	}
	return i;
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
u8_to_i32h(register u8 x)
/*@*/
{
	return (i32) (x - 0x80u);
}

/**@fn pcm_read_i16le
 * @brief reads a sample of i16le PCM into an i32 sample
 *
 * @param dest[out] destination sample
 * @param src[in] source buffer
**/
ALWAYS_INLINE void
read_i16le_to_i32h(register i32 *const dest, register const u8 *const src)
/*@modifies	*dest@*/
{
	*((u32 *) dest)  =  (u32)       src[0u];
	*((u32 *) dest) |= ((u32) ((i8) src[1u])) << 8u;
	return;
}

/**@fn pcm_read_i24le
 * @brief reads a sample of i24le PCM into an i32 sample
 *
 * @param dest[out] destination sample
 * @param src[in] source buffer
**/
ALWAYS_INLINE void
read_i24le_to_i32h(register i32 *const dest, register const u8 *const src)
/*@modifies	*dest@*/
{
	*((u32 *) dest)  =  (u32)       src[0u];
	*((u32 *) dest) |= ((u32)       src[1u])  <<  8u;
	*((u32 *) dest) |= ((u32) ((i8) src[2u])) << 16u;
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
