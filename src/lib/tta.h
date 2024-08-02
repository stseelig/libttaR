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

#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"

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
INLINE CONST u8 get_predict_k(enum TTASampleBytes) /*@*/;
INLINE CONST i32 get_filter_round(enum TTASampleBytes) /*@*/;
INLINE CONST u8 get_filter_k(enum TTASampleBytes) /*@*/;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST i32 asl32(i32, u8) /*@*/;
ALWAYS_INLINE CONST i32 asr32(i32, u8) /*@*/;

//--------------------------------------------------------------------------//

INLINE CONST i32 tta_predict1(i32, u8) /*@*/;
INLINE CONST i32 tta_postfilter_enc(i32) /*@*/;
INLINE CONST i32 tta_prefilter_dec(i32) /*@*/;

//--------------------------------------------------------------------------//

#undef filter
ALWAYS_INLINE i32 tta_filter(
	struct Filter *restrict filter, i32, u8, i32, enum TTAMode
)
/*@modifies	*filter@*/
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
INLINE CONST u8
get_predict_k(const enum TTASampleBytes samplebytes)
/*@*/
{
	u8 r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
		r = (u8) 4u;
		break;
	case TTASAMPLEBYTES_2:
	case TTASAMPLEBYTES_3:
		r = (u8) 5u;
		break;
	}
	return r;
}

/**@fn get_filter_round
 * @brief arg for tta_filter
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'sum' for tta_filter
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
INLINE CONST u8
get_filter_k(const enum TTASampleBytes samplebytes)
/*@*/
{
	u8 r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_3:
		r = (u8) 10u;
		break;
	case TTASAMPLEBYTES_2:
		r = (u8)  9u;
		break;
	}
	return r;
}

//==========================================================================//

// shifting signed integers is naughty (implementation defined)

/**@fn asl32
 * @brief arithmetic shift left 32-bit
 *
 * @param x value to shift
 * @param k amount to shift
 *
 * @return shifted value
**/
ALWAYS_INLINE CONST i32
asl32(const i32 x, const u8 k)
/*@*/
{
	return (i32) (((u32) x) << k);
}

/**@fn asr32
 * @brief arithmetic shift right 32-bit
 *
 * @param x value to shift
 * @param k amount to shift
 *
 * @return shifted value
**/
ALWAYS_INLINE CONST i32
asr32(const i32 x, const u8 k)
/*@*/
{
	/*@-shiftimplementation@*/
	if ( (x < 0) && ((i32) (((i32) -1) >> 1u) != (i32) -1) ){
	/*@=shiftimplementation@*/
		return (i32) ~((~((u32) x)) >> k);
	}
	else {	/*@-shiftimplementation@*/
		return (i32) (x >> k);
		/*@=shiftimplementation@*/
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
**/
ALWAYS_INLINE CONST i32
tta_predict1(const i32 x, const u8 k)
/*@*/
{
	return (i32) (((((u64f) x) << k) - x) >> k);
}

/**@fn tta_postfilter_enc
 * @brief interleave value for coding
 *
 * @param x input value
 *
 * @return interleaved value
 *
 * @note https://en.wikipedia.org/wiki/Golomb_coding#Overview#\
 *     Use%20with%20signed%20integers
**/
ALWAYS_INLINE CONST i32
tta_postfilter_enc(const i32 x)
/*@*/
{
	return (x > 0 ? asl32(x, (u8) 1u) - 1 : asl32(-x, (u8) 1u));
}

/**@fn tta_prefilter_dec
 * @brief deinterleave value for filtering
 *
 * @param x input value
 *
 * @return deinterleaved value
 *
 * @see tta_postfilter_enc
**/
ALWAYS_INLINE CONST i32
tta_prefilter_dec(const i32 x)
/*@*/
{
	return ((((u32) x) & 0x1u) != 0
		? asr32(x + 1, (u8) 1u) : asr32(-x, (u8) 1u)
	);
}

//==========================================================================//

/**@fn tta_filter
 * @brief adaptive hybrid filter
 *
 * @param filter[in out] the filter data for the current channel
 * @param sum intial sum / round
 * @param k amount to shift the 'sum' by before add/subtract-ing from 'value'
 * @param value the input value to filter
 * @param mode encode or decode
 *
 * @return the filtered value
 *
 * @note affected by LIBTTAr_OPT_DISABLE_BRANCHLESS_FILTER
**/
ALWAYS_INLINE i32
tta_filter(
	struct Filter *const restrict filter, i32 sum, const u8 k, i32 value,
	const enum TTAMode mode
)
/*@modifies	*filter@*/
{
	i32 *const restrict a = filter->qm;
	i32 *const restrict m = filter->dx;
	i32 *const restrict b = filter->dl;

#ifndef LIBTTAr_OPT_DISABLE_BRANCHLESS_FILTER
	const i32 e = (filter->error != 0
		? (filter->error < 0 ? (i32) -1 : (i32) 1) : 0
	);
#endif
	uint i;

	// for-loops SIMD better than unrolled
#ifndef LIBTTAr_OPT_DISABLE_BRANCHLESS_FILTER
	for ( i = 0; i < 8u; ++i ){
		sum += (a[i] += m[i] * e) * b[i];
	}
#else
	// there is a compiler quirk where putting the ==0 branch !first slows
	//   everything down considerably. it adds an branch or two to reduce
	//   code size a bit, but that is just slower. logically, the ==0
	//   branch should be last, because it is the least likely to happen
	//   (main exception being silence)
	if ( filter->error == 0 ){
		for ( i = 0; i < 8u; ++i ){
			sum += a[i] * b[i];
		}
	}
	else if ( filter->error < 0 ){
		for ( i = 0; i < 8u; ++i ){
			sum += (a[i] -= m[i]) * b[i];
		}
	}
	else {	// filter->error > 0
		for ( i = 0; i < 8u; ++i ){
			sum += (a[i] += m[i]) * b[i];
		}
	}
#endif
	m[8u] = (i32) ((((u32) asr32(b[7u], (u8) 30u)) | 0x1u) << 2u);
	m[7u] = (i32) ((((u32) asr32(b[6u], (u8) 30u)) | 0x1u) << 1u);
	m[6u] = (i32) ((((u32) asr32(b[5u], (u8) 30u)) | 0x1u) << 1u);
	m[5u] = (i32) ((((u32) asr32(b[4u], (u8) 30u)) | 0x1u) << 0u);

	switch ( mode ){
	case TTA_ENC:
		b[8u]         = value;
		value        -= asr32(sum, k);
		filter->error = value;
		break;
	case TTA_DEC:
		filter->error = value;
		value        += asr32(sum, k);
		b[8u]         = value;
		break;
	}

	b[7u] = b[8u] - b[7u];
	b[6u] = b[7u] - b[6u];
	b[5u] = b[6u] - b[5u];

	MEMMOVE(m, &m[1u], (size_t) (8u*(sizeof *m)));
	MEMMOVE(b, &b[1u], (size_t) (8u*(sizeof *b)));

	return value;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
