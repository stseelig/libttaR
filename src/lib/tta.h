#ifndef TTA_CODEC_TTA_H
#define TTA_CODEC_TTA_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta.h                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <limits.h>	// INT_MIN
#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"
#include "filter.h"

//////////////////////////////////////////////////////////////////////////////

// frame finished
#define LIBTTAr_RET_DONE		 0

// frame not finished
#define LIBTTAr_RET_AGAIN		 1

// frame finished, but (nbytes_tta_total != nbytes_tta_perframe)
//||
// frame not finished, but (nbytes_tta_total > nbytes_tta_perframe)
#define LIBTTAr_RET_DECFAIL		 2

// some misc value is 0 or a bad enum value
#define LIBTTAr_RET_INVAL_RANGE		-1

// (ni32_target % nchan != 0)
#define LIBTTAr_RET_INVAL_TRUNC		-2

// some misc value would cause a bounds issue
#define LIBTTAr_RET_INVAL_BOUNDS	-3

// library was misconfigured; @see libttaR_test_nchan()
#define LIBTTAr_RET_MISCONFIG	SCHAR_MIN

enum LibTTAr_EncRetVal {
	LIBTTAr_ERV_DONE		= LIBTTAr_RET_DONE,
	LIBTTAr_ERV_AGAIN		= LIBTTAr_RET_AGAIN,
	LIBTTAr_ERV_INVAL_RANGE		= LIBTTAr_RET_INVAL_RANGE,
	LIBTTAr_ERV_INVAL_TRUNC		= LIBTTAr_RET_INVAL_TRUNC,
	LIBTTAr_ERV_INVAL_BOUNDS	= LIBTTAr_RET_INVAL_BOUNDS,
	LIBTTAr_ERV_MISCONFIG		= LIBTTAr_RET_MISCONFIG
};

enum LibTTAr_DecRetVal {
	LIBTTAr_DRV_DONE		= LIBTTAr_RET_DONE,
	LIBTTAr_DRV_AGAIN		= LIBTTAr_RET_AGAIN,
	LIBTTAr_DRV_FAIL		= LIBTTAr_RET_DECFAIL,
	LIBTTAr_DRV_INVAL_RANGE		= LIBTTAr_RET_INVAL_RANGE,
	LIBTTAr_DRV_INVAL_TRUNC		= LIBTTAr_RET_INVAL_TRUNC,
	LIBTTAr_DRV_INVAL_BOUNDS	= LIBTTAr_RET_INVAL_BOUNDS,
	LIBTTAr_DRV_MISCONFIG		= LIBTTAr_RET_MISCONFIG
};

// max unary r/w size:		read		write
//	8/16-bit:		   8194u	   8199u
//	  24-bit:		2097154uL	2097159uL
// max binary r/w size:		      3u	      0u
// max cacheflush w size: 		  	      8u
#define TTABUF_SAFETY_MARGIN_1_2	((size_t)    8207u)
#define TTABUF_SAFETY_MARGIN_3		((size_t) 2097167uL)

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE CONST
size_t get_safety_margin(enum LibTTAr_SampleBytes, uint) /*@*/;

ALWAYS_INLINE CONST
bitcnt get_predict_k(enum LibTTAr_SampleBytes) /*@*/;

ALWAYS_INLINE CONST
i32 get_filter_round(enum LibTTAr_SampleBytes) /*@*/;

ALWAYS_INLINE CONST
bitcnt get_filter_k(enum LibTTAr_SampleBytes) /*@*/;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST i32 asr32(i32, bitcnt) /*@*/;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST i32 tta_predict1(i32, bitcnt) /*@*/;
ALWAYS_INLINE CONST u32 tta_postfilter_enc(i32) /*@*/;
ALWAYS_INLINE CONST i32 tta_prefilter_dec(u32) /*@*/;

//////////////////////////////////////////////////////////////////////////////

// undefined behavior assertions for the codec functions
#define CODEC_UB_ASSERTS_PRE { \
	assert(((uintptr_t) &priv->codec) % STRUCT_CODEC_ALIGNMENT == 0); \
	SAMPLEBYTES_RANGE_ASSERT(misc->samplebytes); \
}

//////////////////////////////////////////////////////////////////////////////

/**@fn get_safety_margin
 * @brief safety margin for the TTA buffer
 *
 * @param samplebytes number of bytes per PCM sample
 * @param nchan number of audio channels
 *
 * @return safety margin
**/
ALWAYS_INLINE CONST size_t
get_safety_margin(
	const enum LibTTAr_SampleBytes samplebytes, const uint nchan
)
/*@*/
{
	SAMPLEBYTES_RANGE_ASSERT(samplebytes);

	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
	case LIBTTAr_SAMPLEBYTES_2:
		return (size_t) (nchan * TTABUF_SAFETY_MARGIN_1_2);
	case LIBTTAr_SAMPLEBYTES_3:
		return (size_t) (nchan * TTABUF_SAFETY_MARGIN_3);
	}
	UNREACHABLE;
}

/**@fn get_predict_k
 * @brief arg for tta_predict1
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'k' for tta_predict1
**/
ALWAYS_INLINE CONST bitcnt
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
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'round' for tta_filter
**/
ALWAYS_INLINE CONST i32
get_filter_round(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	SAMPLEBYTES_RANGE_ASSERT(samplebytes);

	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
	case LIBTTAr_SAMPLEBYTES_3:
		return (i32) 0x00000200;	// binexp32(filter_k - 1u)
	case LIBTTAr_SAMPLEBYTES_2:
		return (i32) 0x00000100;	// ~
	}
	UNREACHABLE;
}

/**@fn get_filter_k
 * @brief arg for tta_filter
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'k' for tta_filter
**/
ALWAYS_INLINE CONST bitcnt
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

//==========================================================================//

// checks if the targeted arch has an signed (arithmetic) right shift
#define HAS_ASR(Xtype)	( \
	/*@-shiftimplementation@*/ \
	(Xtype) (((Xtype) UINTMAX_MAX) >> 1u) == (Xtype) UINTMAX_MAX \
	/*@=shiftimplementation@*/ \
)

/**@fn asr32
 * @brief arithmetic shift right 32-bit
 *
 * @param x value to shift
 * @param k amount to shift
 *
 * @return shifted value
 *
 * @pre k <= (bitcnt) 31u
**/
ALWAYS_INLINE CONST i32
asr32(const i32 x, const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 31u);

	if ( HAS_ASR(i32) ){
		/*@-shiftimplementation@*/
		return (i32) (x >> k);
		/*@=shiftimplementation@*/
	}
	else {	return (UNPREDICTABLE (x < 0)
			? (i32) ~((~((u32) x)) >> k) : (i32) (((u32) x) >> k)
		);
	}
}

//==========================================================================//

/**@fn tta_predict1
 * @brief fixed order 1 prediction
 *
 * @param x input value
 * @param k how much to shift it by
 *
 * @return predicted value
 *
 * @pre k <= (bitcnt) 32u
**/
ALWAYS_INLINE CONST i32
tta_predict1(const i32 x, const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 32u);

	return (i32) (((((u64f) x) << k) - x) >> k);
}

/**@fn tta_postfilter_enc
 * @brief interleave value for coding
 *        0 => 0
 *        1 => 1
 *       -1 => 2
 *        2 => 3
 *       -2 => 4
 *
 * @param x input value
 *
 * @return interleaved value
 *
 * @note https://en.wikipedia.org/wiki/Golomb_coding#Overview#\
 *     Use%20with%20signed%20integers
 * @note affected by LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
**/
ALWAYS_INLINE CONST u32
tta_postfilter_enc(const i32 x)
/*@*/
{
#ifndef LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
	const u32 y     = -((u32) x);
	const u32 xsign = (u32) asr32((i32) y, (bitcnt) 31u);
	return (u32) ((y << 1u) ^ xsign);
#else
	const u32 yp = (u32) x, yn = -((u32) x);
	return (UNPREDICTABLE (x > 0)
		? (u32) ((yp << 1u) - 1u) : (u32) (yn << 1u)
	);
#endif
}

/**@fn tta_prefilter_dec
 * @brief deinterleave value for filtering
 *        0 =>  0
 *        1 =>  1
 *        2 => -1
 *        3 =>  2
 *        4 => -2
 *
 * @param x input value
 *
 * @return deinterleaved value
 *
 * @see tta_postfilter_enc
 * @note affected by LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
**/
ALWAYS_INLINE CONST i32
tta_prefilter_dec(const u32 x)
/*@*/
{
#ifndef LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
	const u32 xsign = (u32) -(x & 0x1u);
	return (i32) -((x >> 1u) ^ xsign);
#else
	const i32 y = (i32) x;
	return (UNPREDICTABLE ((((u32) x) & 0x1u) != 0)
		? asr32(y + 1, (bitcnt) 1u)
		: asr32((i32) -((u32) y), (bitcnt) 1u)
	);
#endif
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
