#ifndef TTA_CODEC_TTA_H
#define TTA_CODEC_TTA_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta.h                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"
#include "filter.h"

//////////////////////////////////////////////////////////////////////////////

enum LibTTAr_RetVal {
	// frame finished
	LIBTTAr_RET_DONE	 =  0,

	// frame not finished
	LIBTTAr_RET_AGAIN	 =  1,

	// frame finished, but (nbytes_tta_total != nbytes_tta_perframe)
	//||
	// frame not finished, but (nbytes_tta_total > nbytes_tta_perframe)
	LIBTTAr_RET_DECFAIL      =  2,

	// (ni32_target % nchan != 0) or other bad parameter
	// used as the base value, can return greater values
	LIBTTAr_RET_INVAL,

	// library was misconfigured; see libttaR_test_nchan
	LIBTTAr_RET_MISCONFIG	 = -1
};

// max unary r/w size:		read	write
//	8/16-bit:		  34u	  40u
//	  24-bit:		4098u	4104u
// max binary r/w size:		   3u	   0u
// max cacheflush w size: 		   8u
// rounded up to the nearest (power of 2) + (power of 2)
#define TTABUF_SAFETY_MARGIN_1_2	((size_t)   48u)
#define TTABUF_SAFETY_MARGIN_3		((size_t) 4112u)

//////////////////////////////////////////////////////////////////////////////

INLINE CONST size_t get_safety_margin(enum TTASampleBytes, uint) /*@*/;
INLINE CONST bitcnt get_predict_k(enum TTASampleBytes) /*@*/;
INLINE CONST i32 get_filter_round(enum TTASampleBytes) /*@*/;
INLINE CONST bitcnt get_filter_k(enum TTASampleBytes) /*@*/;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST i32 asr32(i32, bitcnt) /*@*/;
ALWAYS_INLINE CONST i32 signof32(i32) /*@*/;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST i32 tta_predict1(i32, bitcnt) /*@*/;
ALWAYS_INLINE CONST u32 tta_postfilter_enc(i32) /*@*/;
ALWAYS_INLINE CONST i32 tta_prefilter_dec(u32) /*@*/;

//--------------------------------------------------------------------------//

#undef filter
ALWAYS_INLINE i32 tta_filter_enc(
	struct Filter *restrict filter, i32, i32, bitcnt
)
/*@modifies	*filter@*/
;

#undef filter
ALWAYS_INLINE i32 tta_filter_dec(
	struct Filter *restrict filter, i32, i32, bitcnt
)
/*@modifies	*filter@*/
;

#undef a
ALWAYS_INLINE i32 filter_sum(
	i32 *restrict a, const i32 *restrict, const i32 *restrict , i32, i32
)
/*@modifies	*a@*/
;

#undef m
ALWAYS_INLINE void filter_update_m(i32 *restrict m, const i32 *restrict)
/*@modifies	*m@*/
;

ALWAYS_INLINE CONST i32 updated_m(i32, bitcnt) /*@*/;

#undef b
ALWAYS_INLINE void filter_update_b(i32 *restrict b)
/*@modifies	*b@*/
;

#undef m
#undef b
ALWAYS_INLINE void filter_shift(i32 *restrict m, i32 *restrict b)
/*@modifies	*m,
		*b
@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn get_safety_margin
 * @brief safety margin for the TTA buffer
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return safety margin
**/
INLINE CONST size_t
get_safety_margin(const enum TTASampleBytes samplebytes, const uint nchan)
/*@*/
{
	size_t r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_2:
		r = TTABUF_SAFETY_MARGIN_1_2;
		break;
	case TTASAMPLEBYTES_3:
		r = TTABUF_SAFETY_MARGIN_3;
		break;
	}
	return (size_t) (r * nchan);
}

/**@fn get_predict_k
 * @brief arg for tta_predict1
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'k' for tta_predict1
**/
INLINE CONST bitcnt
get_predict_k(const enum TTASampleBytes samplebytes)
/*@*/
{
	bitcnt r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
		r = (bitcnt) 4u;
		break;
	case TTASAMPLEBYTES_2:
	case TTASAMPLEBYTES_3:
		r = (bitcnt) 5u;
		break;
	}
	return r;
}

/**@fn get_filter_round
 * @brief arg for tta_filter
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'round' for tta_filter
**/
INLINE CONST i32
get_filter_round(const enum TTASampleBytes samplebytes)
/*@*/
{
	i32 r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_3:
		r = (i32) 0x00000200;	// binexp32(filter_k - 1u)
		break;
	case TTASAMPLEBYTES_2:
		r = (i32) 0x00000100;	// ~
		break;
	}
	return r;
}

/**@fn get_filter_k
 * @brief arg for tta_filter
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'k' for tta_filter
**/
INLINE CONST bitcnt
get_filter_k(const enum TTASampleBytes samplebytes)
/*@*/
{
	bitcnt r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_3:
		r = (bitcnt) 10u;
		break;
	case TTASAMPLEBYTES_2:
		r = (bitcnt)  9u;
		break;
	}
	return r;
}

//==========================================================================//

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

	/*@-shiftimplementation@*/
	if ( (i32) (((i32) -1) >> 1u) == (i32) -1 ){
		// native
		return (i32) (x >> k);
	}
	else {	// emulated
		return (UNPREDICTABLE (x < 0)
			? (i32) ~((~((u32) x)) >> k) : (i32) (x >> k)
		);
	}
	/*@=shiftimplementation@*/
}

/**@fn signof32
 * @brief get the sign of a 32-bit integer
 *
 * @param x input value
 *
 * @retval -1, 1, or 0
 *
 * @note affected by LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
**/
ALWAYS_INLINE CONST i32
signof32(const i32 x)
/*@*/
{
#ifndef LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
	const u32 y = (u32) -x;
	return (i32) (asr32(x, (bitcnt) 31u) | (y >> 31u));
#else
	return (UNPREDICTABLE (x != 0)
		? (UNPREDICTABLE (x < 0) ? (i32) -1 : (i32) 1) : 0
	);
#endif
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
	const u32 y     = (u32) -x;
	const u32 xsign = (u32) asr32((i32) y, (bitcnt) 31u);
	return (u32) ((y << 1u) ^ xsign);
#else
	const u32 yp = (u32) x, yn = (u32) -x;
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
	const u32 xsign = (u32) -((i32) (x & 0x1u));
	return -((i32) ((x >> 1u) ^ xsign));
#else
	const i32 y = (i32) x;
	return (UNPREDICTABLE ((((u32) x) & 0x1u) != 0)
		? asr32(y + 1, (bitcnt) 1u) : asr32(-y, (bitcnt) 1u)
	);
#endif
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
