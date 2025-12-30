#ifndef H_TTA_CODEC_TTA_H
#define H_TTA_CODEC_TTA_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta.h                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "./common.h"
#include "./overflow.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

/* frame finished */
#define X_LIBTTAr_RV_OK_DONE		 0

/* frame not finished */
#define X_LIBTTAr_RV_OK_AGAIN		 1

/* frame finished, but (nbytes_tta_total != nbytes_tta_perframe)
  ||
   frame not finished, but (nbytes_tta_total > nbytes_tta_perframe)
*/
#define X_LIBTTAr_RV_FAIL_DECODE	 2

/* some 'user' value overflowed */
#define X_LIBTTAr_RV_FAIL_OVERFLOW	 3

/* some misc value is 0 or a bad enum value */
#define X_LIBTTAr_RV_INVAL_RANGE	-1

/* (ni32_target % nchan != 0) */
#define X_LIBTTAr_RV_INVAL_TRUNC	-2

/* some misc value would cause a bounds issue */
#define X_LIBTTAr_RV_INVAL_BOUNDS	-3

/* a pointer/buffer is not properly aligned */
#define X_LIBTTAr_RV_INVAL_ALIGN	-4

/* library was misconfigured; @see libttaR_test_nchan() */
#define X_LIBTTAr_RV_MISCONFIG		INT8_MIN

enum LibTTAr_EncRetVal {
	LIBTTAr_ERV_OK_DONE		= X_LIBTTAr_RV_OK_DONE,
	LIBTTAr_ERV_OK_AGAIN		= X_LIBTTAr_RV_OK_AGAIN,
	LIBTTAr_ERV_FAIL_OVERFLOW	= X_LIBTTAr_RV_FAIL_OVERFLOW,
	LIBTTAr_ERV_INVAL_RANGE		= X_LIBTTAr_RV_INVAL_RANGE,
	LIBTTAr_ERV_INVAL_TRUNC		= X_LIBTTAr_RV_INVAL_TRUNC,
	LIBTTAr_ERV_INVAL_BOUNDS	= X_LIBTTAr_RV_INVAL_BOUNDS,
	LIBTTAr_ERV_INVAL_ALIGN		= X_LIBTTAr_RV_INVAL_ALIGN,
	LIBTTAr_ERV_MISCONFIG		= X_LIBTTAr_RV_MISCONFIG
};

enum LibTTAr_DecRetVal {
	LIBTTAr_DRV_OK_DONE		= X_LIBTTAr_RV_OK_DONE,
	LIBTTAr_DRV_OK_AGAIN		= X_LIBTTAr_RV_OK_AGAIN,
	LIBTTAr_DRV_FAIL_DECODE		= X_LIBTTAr_RV_FAIL_DECODE,
	LIBTTAr_DRV_FAIL_OVERFLOW	= X_LIBTTAr_RV_FAIL_OVERFLOW,
	LIBTTAr_DRV_INVAL_RANGE		= X_LIBTTAr_RV_INVAL_RANGE,
	LIBTTAr_DRV_INVAL_TRUNC		= X_LIBTTAr_RV_INVAL_TRUNC,
	LIBTTAr_DRV_INVAL_BOUNDS	= X_LIBTTAr_RV_INVAL_BOUNDS,
	LIBTTAr_DRV_INVAL_ALIGN		= X_LIBTTAr_RV_INVAL_ALIGN,
	LIBTTAr_DRV_MISCONFIG		= X_LIBTTAr_RV_MISCONFIG
};

/* max unary r/w size:		read		write
	8/16-bit:		   8194u	   8199u
	  24-bit:		2097154uL	2097159uL
   max binary r/w size:		      3u	      0u
   max cacheflush w size: 		  	      8u
*/
#define TTABUF_SAFETY_MARGIN_1_2	SIZE_C(   8207)
#define TTABUF_SAFETY_MARGIN_3		SIZE_C(2097167)

/* //////////////////////////////////////////////////////////////////////// */

CONST
ALWAYS_INLINE size_t get_safety_margin(enum LibTTAr_SampleBytes, unsigned int)
/*@*/
;

CONST
ALWAYS_INLINE bitcnt get_predict_k(enum LibTTAr_SampleBytes) /*@*/;

CONST
ALWAYS_INLINE int32_t get_filter_round(enum LibTTAr_SampleBytes) /*@*/;

CONST
ALWAYS_INLINE bitcnt get_filter_k(enum LibTTAr_SampleBytes) /*@*/;

/* ------------------------------------------------------------------------ */

CONST
ALWAYS_INLINE int32_t asr32(int32_t, bitcnt) /*@*/;

/* ------------------------------------------------------------------------ */

CONST
ALWAYS_INLINE int32_t tta_predict1(int32_t, bitcnt) /*@*/;

CONST
ALWAYS_INLINE uint32_t tta_postfilter_enc(int32_t) /*@*/;

CONST
ALWAYS_INLINE int32_t tta_prefilter_dec(uint32_t) /*@*/;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn get_safety_margin
 * @brief safety margin for the TTA buffer
 *
 * @param samplebytes - number of bytes per PCM sample
 * @param nchan       - number of audio channels
 *
 * @return safety margin
 * @retval 0 - (nchan == 0) or overflow
**/
CONST
ALWAYS_INLINE size_t
get_safety_margin(
	const enum LibTTAr_SampleBytes samplebytes, const unsigned int nchan
)
/*@*/
{
	size_t margin;
	int overflow;

	SAMPLEBYTES_RANGE_ASSERT(samplebytes);

	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
	case LIBTTAr_SAMPLEBYTES_2:
		margin = TTABUF_SAFETY_MARGIN_1_2;
		break;
	case LIBTTAr_SAMPLEBYTES_3:
		margin = TTABUF_SAFETY_MARGIN_3;
		break;
	default:
		UNREACHABLE;
	}

	overflow = mul_usize_overflow(&margin, margin, (size_t) nchan);

	return (overflow == 0 ? margin : 0);
}

/**@fn get_predict_k
 * @brief arg for tta_predict1
 *
 * @param samplebytes - number of bytes per PCM sample
 *
 * @return arg 'k' for tta_predict1
**/
CONST
ALWAYS_INLINE bitcnt
get_predict_k(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	SAMPLEBYTES_RANGE_ASSERT(samplebytes);

	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_2:
	case LIBTTAr_SAMPLEBYTES_3:
		return (bitcnt) 5u;
	case LIBTTAr_SAMPLEBYTES_1:
		return (bitcnt) 4u;
	}
	UNREACHABLE;
}

/**@fn get_filter_round
 * @brief arg for tta_filter
 *
 * @param samplebytes - number of bytes per PCM sample
 *
 * @return arg 'round' for tta_filter
**/
CONST
ALWAYS_INLINE int32_t
get_filter_round(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	SAMPLEBYTES_RANGE_ASSERT(samplebytes);

	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
	case LIBTTAr_SAMPLEBYTES_3:
		return INT32_C(0x00000200);	/* binexp32(filter_k - 1u) */
	case LIBTTAr_SAMPLEBYTES_2:
		return INT32_C(0x00000100);	/* ~                       */
	}
	UNREACHABLE;
}

/**@fn get_filter_k
 * @brief arg for tta_filter
 *
 * @param samplebytes - number of bytes per PCM sample
 *
 * @return arg 'k' for tta_filter
**/
CONST
ALWAYS_INLINE bitcnt
get_filter_k(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	SAMPLEBYTES_RANGE_ASSERT(samplebytes);

	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
	case LIBTTAr_SAMPLEBYTES_3:
		return (bitcnt) 10u;
	case LIBTTAr_SAMPLEBYTES_2:
		return (bitcnt)  9u;
	}
	UNREACHABLE;
}

/* ======================================================================== */

/* checks if the targeted arch has an signed (arithmetic) right shift */
#define HAS_ASR(x_type)	( \
	/*@-shiftimplementation@*/ \
	(x_type) (((x_type) UINTMAX_MAX) >> 1u) == (x_type) UINTMAX_MAX \
	/*@=shiftimplementation@*/ \
)

/**@fn asr32
 * @brief arithmetic shift right 32-bit
 *
 * @param x - value to shift
 * @param k - amount to shift
 *
 * @return shifted value
**/
CONST
ALWAYS_INLINE int32_t
asr32(const int32_t x, const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 31u);

	if ( HAS_ASR(int32_t) ){
		/*@-shiftimplementation@*/
		return (int32_t) (x >> k);
		/*@=shiftimplementation@*/
	}
	else {	return (UNPREDICTABLE (x < 0)
			? (int32_t) ~((~((uint32_t) x)) >> k)
			: (int32_t) (((uint32_t) x) >> k)
		);
	}
}

/* ======================================================================== */

/**@fn tta_predict1
 * @brief fixed order 1 prediction
 *
 * @param x - input value
 * @param k - how much to shift it by
 *
 * @return predicted value
**/
CONST
ALWAYS_INLINE int32_t
tta_predict1(const int32_t x, const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 32u);

	return (int32_t) (((((uint_fast64_t) x) << k) - x) >> k);
}

/**@fn tta_postfilter_enc
 * @brief interleave value for coding
 *        0 => 0
 *        1 => 1
 *       -1 => 2
 *        2 => 3
 *       -2 => 4
 *
 * @param x - input value
 *
 * @return interleaved value
 *
 * @note the function only needs to work for 24-bit integers, so there should
 *   be no need to handle INT32_MIN in the branchless version
 * @note https://en.wikipedia.org/wiki/Golomb_coding#Overview#\
 *     Use%20with%20signed%20integers
**/
CONST
ALWAYS_INLINE uint32_t
tta_postfilter_enc(const int32_t x)
/*@*/
{
#ifndef LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES

	const uint32_t y     = -((uint32_t) x);
	const uint32_t xsign = (uint32_t) asr32((int32_t) y, (bitcnt) 31u);

	return (uint32_t) ((y << 1u) ^ xsign);

#else	/* defined(LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES) */

	const uint32_t yp    =   (uint32_t) x;
	const uint32_t yn    = -((uint32_t) x);

	return (UNPREDICTABLE (x > 0)
		? (uint32_t) ((yp << 1u) - 1u) : (uint32_t) (yn << 1u)
	);

#endif	/* LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES */
}

/**@fn tta_prefilter_dec
 * @brief deinterleave value for filtering
 *        0 =>  0
 *        1 =>  1
 *        2 => -1
 *        3 =>  2
 *        4 => -2
 *
 * @param x - input value
 *
 * @return deinterleaved value
 *
 * @see tta_postfilter_enc
**/
CONST
ALWAYS_INLINE int32_t
tta_prefilter_dec(const uint32_t x)
/*@*/
{
#ifndef LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES

	const uint32_t xsign = (uint32_t) -(x & 0x1u);

	return (int32_t) -((x >> 1u) ^ xsign);

#else	/* defined(LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES) */

	const int32_t y = (int32_t) x;

	return (UNPREDICTABLE ((((uint32_t) x) & 0x1u) != 0)
		? asr32(y + 1, (bitcnt) 1u)
		: asr32((int32_t) -((uint32_t) y), (bitcnt) 1u)
	);

#endif	/* LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES */
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_TTA_H */
