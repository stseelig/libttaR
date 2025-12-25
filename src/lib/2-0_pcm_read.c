/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_read.c                                                         //
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
static NOINLINE size_t pcm_read_u8(
	/*@out@*/ int32_t *RESTRICT dest, const uint8_t *RESTRICT, size_t
)
/*@modifies	*dest@*/
;

#undef dest
static NOINLINE size_t pcm_read_i16le(
	/*@out@*/ int32_t *RESTRICT dest, const uint8_t *RESTRICT, size_t
)
/*@modifies	*dest@*/
;

#undef dest
static NOINLINE size_t pcm_read_i24le(
	/*@out@*/ int32_t *RESTRICT dest, const uint8_t *RESTRICT, size_t
)
/*@modifies	*dest@*/
;

/* ------------------------------------------------------------------------ */

CONST
ALWAYS_INLINE int32_t uint8_t_to_i32h(uint8_t) /*@*/;

PURE
ALWAYS_INLINE int32_t read_i16le_to_i32h(const uint8_t *RESTRICT) /*@*/;

PURE
ALWAYS_INLINE int32_t read_i24le_to_i32h(const uint8_t *RESTRICT) /*@*/;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn libttaR_pcm_read
 * @brief reads a buffer of PCM into a buffer of int32_t
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
libttaR_pcm_read(
	/*@out@*/ int32_t *const RESTRICT dest,
	const uint8_t *const RESTRICT src, const size_t nsamples,
	const enum LibTTAr_SampleBytes samplebytes
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

/* ------------------------------------------------------------------------ */

/**@fn pcm_read_u8
 * @brief reads a buffer of uint8_t PCM into a buffer of int32_t
 *
 * @param dest     - destination buffer
 * @param src      - source buffer
 * @param nsamples - total number of PCM samples
 *
 * @return 'nsamples'
**/
static NOINLINE size_t
pcm_read_u8(
	/*@out@*/ int32_t *const RESTRICT dest,
	const uint8_t *const RESTRICT src, const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i;

	for ( i = 0; i < nsamples; ++i ){
		dest[i] = uint8_t_to_i32h(src[i]);
	}
	/*@-mustdefine@*/
	return i;
	/*@=mustdefine@*/
}

/**@fn pcm_read_i16le
 * @brief reads a buffer of i16le PCM into a buffer of int32_t
 *
 * @param dest     - destination buffer
 * @param src      - source buffer
 * @param nsamples - total number of PCM samples
 *
 * @return 'nsamples'
**/
static NOINLINE size_t
pcm_read_i16le(
	/*@out@*/ int32_t *const RESTRICT dest,
	const uint8_t *const RESTRICT src, const size_t nsamples
)
/*@modifies	*dest@*/
{
	size_t i, j;

	for ( i = 0, j = 0; i < nsamples; ++i, j += 2u ){
		dest[i] = read_i16le_to_i32h(&src[j]);
	}
	/*@-mustdefine@*/
	return i;
	/*@=mustdefine@*/
}

/**@fn pcm_read_i24le
 * @brief reads a buffer of i24le PCM into a buffer of int32_t
 *
 * @param dest     - destination buffer
 * @param src      - source buffer
 * @param nsamples - total number of PCM samples
 *
 * @return 'nsamples'
**/
static NOINLINE size_t
pcm_read_i24le(
	/*@out@*/ int32_t *const RESTRICT dest,
	const uint8_t *const RESTRICT src, const size_t nsamples
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

/* ------------------------------------------------------------------------ */

/**@fn uint8_t_to_i32h
 * @brief converts a uint8_t PCM sample into an int32_t sample
 *
 * @param x - u8 sample
 *
 * @return i32 sample
**/
CONST
ALWAYS_INLINE int32_t
uint8_t_to_i32h(const uint8_t x)
/*@*/
{
	return (int32_t) (x - UINT8_C(0x80));
}

/**@fn read_i16le_to_i32h
 * @brief reads a sample of i16le PCM into an int32_t sample
 *
 * @param src - source buffer
 *
 * @return i32 sample
**/
PURE
ALWAYS_INLINE int32_t
read_i16le_to_i32h(const uint8_t *const RESTRICT src)
/*@*/
{
	uint32_t retval = 0;

	retval |=  (uint32_t)           src[0u];
	retval |= ((uint32_t) ((int8_t) src[1u])) << 8u;

	return (int32_t) retval;
}

/**@fn read_i24le_to_i32h
 * @brief reads a sample of i24le PCM into an int32_t sample
 *
 * @param src - source buffer
 *
 * @return i32 sample
**/
PURE
ALWAYS_INLINE int32_t
read_i24le_to_i32h(const uint8_t *const RESTRICT src)
/*@*/
{
	uint32_t retval = 0;

	retval |=  (uint32_t)           src[0u];
	retval |= ((uint32_t)           src[1u])  <<  8u;
	retval |= ((uint32_t) ((int8_t) src[2u])) << 16u;

	return (int32_t) retval;
}

/* EOF //////////////////////////////////////////////////////////////////// */
