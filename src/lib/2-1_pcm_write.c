/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_write.c                                                        //
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
/////////////////////////////////////////////////////////////////////////// */

#include <stddef.h>
#include <stdint.h>

#include "./common.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef dest
static NOINLINE size_t pcm_write_u8(
	/*@out@*/ uint8_t *RESTRICT dest, const int32_t *RESTRICT, size_t
)
/*@modifies	*dest@*/
;

#undef dest
static NOINLINE size_t pcm_write_i16le(
	/*@out@*/ uint8_t *RESTRICT dest, const int32_t *RESTRICT, size_t
)
/*@modifies	*dest@*/
;

#undef dest
static NOINLINE size_t pcm_write_i24le(
	/*@out@*/ uint8_t *RESTRICT dest, const int32_t *RESTRICT, size_t
)
/*@modifies	*dest@*/
;

/* ------------------------------------------------------------------------ */

CONST
ALWAYS_INLINE uint8_t i32h_to_u8(int32_t) /*@*/;

#undef dest
ALWAYS_INLINE void write_i32h_to_i16le(
	/*@out@*/ uint8_t *RESTRICT dest, int32_t
)
/*@modifies	*dest@*/
;

#undef dest
ALWAYS_INLINE void write_i32h_to_i24le(
	/*@out@*/ uint8_t *RESTRICT dest, int32_t
)
/*@modifies	*dest@*/
;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn libttaR_pcm_write
 * @brief reads a buffer of i32 into a buffer of PCM
 *
 * @param dest        - destination buffer
 * @param src         - source buffer
 * @param nsamples    - total number of PCM samples
 * @param samplebytes - number of bytes per PCM sample
 *
 * @return 'nsamples' on success
 * @retval 0 - error (nsamples == 0, or bad enum value)
 *
 * @note read the manpage for more info
**/
BUILD_EXPORT
size_t
libttaR_pcm_write(
	/*@out@*/ uint8_t *const RESTRICT dest,
	const int32_t *const RESTRICT src, const size_t nsamples,
	const enum LibTTAr_SampleBytes samplebytes
)
/*@modifies	*dest@*/
{
	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
		return pcm_write_u8(dest, src, nsamples);
	case LIBTTAr_SAMPLEBYTES_2:
		return pcm_write_i16le(dest, src, nsamples);
	case LIBTTAr_SAMPLEBYTES_3:
		return pcm_write_i24le(dest, src, nsamples);
	}
	/*@notreached@*/
	return 0;
}

/* ------------------------------------------------------------------------ */

/**@fn pcm_write_u8
 * @brief writes a buffer of int32_t into a buffer of uint8_t PCM
 *
 * @param dest[    - destination buffer
 * @param src      - source buffer
 * @param nsamples - total number of PCM samples
 *
 * @return 'nsamples'
**/
static NOINLINE size_t
pcm_write_u8(
	/*@out@*/ uint8_t *const RESTRICT dest,
	const int32_t *const RESTRICT src, const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i;

	for ( i = 0; i < nsamples; ++i ){
		dest[i] = i32h_to_u8(src[i]);
	}
	/*@-mustdefine@*/
	return i;
	/*@=mustdefine@*/
}

/**@fn pcm_write_i16le
 * @brief writes a buffer of i32 into a buffer of i16le PCM
 *
 * @param dest     -  destination buffer
 * @param src      - source buffer
 * @param nsamples - total number of PCM samples
 *
 * @return 'nsamples'
**/
static NOINLINE size_t
pcm_write_i16le(
	/*@out@*/ uint8_t *const RESTRICT dest,
	const int32_t *const RESTRICT src, const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i, j;

	for ( i = 0, j = 0; i < nsamples; ++i, j += 2u ){
		write_i32h_to_i16le(&dest[j], src[i]);
	}
	/*@-mustdefine@*/
	return i;
	/*@=mustdefine@*/
}

/**@fn pcm_write_i24le
 * @brief writes a buffer of i32 into a buffer of i24le PCM
 *
 * @param dest     - destination buffer
 * @param src      - source buffer
 * @param nsamples - total number of PCM samples
 *
 * @return 'nsamples'
**/
static NOINLINE size_t
pcm_write_i24le(
	/*@out@*/ uint8_t *const RESTRICT dest,
	const int32_t *const RESTRICT src, const size_t nsamples
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

/* ------------------------------------------------------------------------ */

/**@fn i32h_to_u8
 * @brief converts an int32_t sample from a u8 PCM sample
 *
 * @param x - u8 sample
 *
 * @return i32 sample
**/
CONST
ALWAYS_INLINE uint8_t
i32h_to_u8(const int32_t x)
/*@*/
{
	return ((uint8_t) x) + UINT8_C(0x80);
}

/**@fn write_i32h_to_i16le
 * @brief writes a sample of i16le PCM from an i32 sample
 *
 * @param dest - destination sample
 * @param src  - source buffer
**/
ALWAYS_INLINE void
write_i32h_to_i16le(/*@out@*/ uint8_t *const RESTRICT dest, const int32_t x)
/*@modifies	*dest@*/
{
	dest[0u] = (uint8_t)  ((uint32_t) x);
	dest[1u] = (uint8_t) (((uint32_t) x) >> 8u);

	return;
}

/**@fn write_i32h_to_i24le
 * @brief writes a sample of i24le PCM from an i32 sample
 *
 * @param dest - destination sample
 * @param src  - source buffer
**/
ALWAYS_INLINE void
write_i32h_to_i24le(/*@out@*/ uint8_t *const RESTRICT dest, const int32_t x)
/*@modifies	*dest@*/
{
	dest[0u] = (uint8_t)  ((uint32_t) x);
	dest[1u] = (uint8_t) (((uint32_t) x) >>  8u);
	dest[2u] = (uint8_t) (((uint32_t) x) >> 16u);

	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
