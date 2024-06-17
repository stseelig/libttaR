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

#include "rice.h"	// struct Rice, shift32_bit

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

enum TTASampleBytes {
	TTASAMPLEBYTES_1 = 1u,
	TTASAMPLEBYTES_2 = 2u,
	TTASAMPLEBYTES_3 = 3u
};
#define TTA_SAMPLEBYTES_MAX	((uint) TTASAMPLEBYTES_3)
#define TTA_SAMPLEBITS_MAX	((uint) (8u*TTA_SAMPLEBYTES_MAX))

// max unary code size
//	 8-bit : 16u
//	16-bit : 16u
//	24-bit : 4096u
// max binary code size : 3u
// max cacheflush size  : 4u
//
// rounded up to the nearest (power of 2) + (power of 2)
#define TTABUF_SAFETY_MARGIN_1or2byte	((size_t)   24u)
#define TTABUF_SAFETY_MARGIN_3byte	((size_t) 5004u)

//////////////////////////////////////////////////////////////////////////////

enum TTAMode {
	TTA_ENC,
	TTA_DEC
};

//////////////////////////////////////////////////////////////////////////////

struct Filter {
	i32	error;
	i32	qm[8u];
	i32	dx[9u];
	i32	dl[9u];
};

struct Codec {
	struct Filter	filter;
	struct Rice	rice;
	i32		prev;
};

//////////////////////////////////////////////////////////////////////////////

#undef codec
INLINE void codec_init(
	/*@out@*/ register struct Codec *const restrict codec , register uint
)
/*@modifies	*codec@*/
;

//--------------------------------------------------------------------------//

INLINE CONST u8 tta_predict_k(register enum TTASampleBytes) /*@*/;
INLINE CONST i32 tta_filter_round(register enum TTASampleBytes) /*@*/;
INLINE CONST u8 tta_filter_k(register enum TTASampleBytes) /*@*/;

//--------------------------------------------------------------------------//

INLINE CONST size_t safety_margin_perchan(register enum TTASampleBytes) /*@*/;
INLINE CONST i32 tta_predict1(register i32, register u8) /*@*/;
INLINE CONST i32 tta_postfilter_enc(register i32) /*@*/;
INLINE CONST i32 tta_prefilter_dec(register i32) /*@*/;

//--------------------------------------------------------------------------//

#undef filter
ALWAYS_INLINE i32 tta_filter(
	register struct Filter *const restrict filter, register i32,
	register u8, register i32, const enum TTAMode
)
/*@modifies	*filter@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn codec_init
 * @brief initializes an array of 'struct Codec'
 *
 * @param codec[out] the struct array to initialize
 * @param nchan number of audio channels
**/
INLINE void
codec_init(
	/*@out@*/ register struct Codec *const restrict codec,
	register uint nchan
)
/*@modifies	*codec@*/
{
	register uint i;
	for ( i = 0; i < nchan; ++i ){
		MEMSET(&codec[i].filter, 0x00, sizeof codec[i].filter);
		rice_init(&codec[i].rice, (u8) 10u, (u8) 10u);
		codec[i].prev = 0;
	}
	return;
}

//--------------------------------------------------------------------------//

/**@fn safety_margin_perchan
 * @brief per channel safety margin for the TTA buffer
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return per channel safety margin
**/
INLINE CONST size_t
safety_margin_perchan(register enum TTASampleBytes samplebytes)
/*@*/
{
	register size_t r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_2:
		r = TTABUF_SAFETY_MARGIN_1or2byte;
		break;
	case TTASAMPLEBYTES_3:
		r = TTABUF_SAFETY_MARGIN_3byte;
		break;
	}
	return r;
}

/**@fn tta_predict_k
 * @brief arg for tta_predict1
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'k' for tta_predict1
**/
INLINE CONST u8
tta_predict_k(register enum TTASampleBytes samplebytes)
/*@*/
{
	register u8 r;
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

/**@fn tta_filter_round
 * @brief arg for tta_filter
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'round' for tta_filter
**/
INLINE CONST i32
tta_filter_round(register enum TTASampleBytes samplebytes)
/*@*/
{
	register i32 r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_3:
		r = (i32) shift32_bit((u8) 9u);
		break;
	case TTASAMPLEBYTES_2:
		r = (i32) shift32_bit((u8) 8u);
		break;
	}
	return r;
}

/**@fn tta_filter_k
 * @brief arg for tta_filter
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return arg 'k' for tta_filter
**/
INLINE CONST u8
tta_filter_k(register enum TTASampleBytes samplebytes)
/*@*/
{
	register u8 r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_3:
		r = (u8) 10u;
		break;
	case TTASAMPLEBYTES_2:
		r = (u8) 9u;
		break;
	}
	return r;
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
tta_predict1(register i32 x, register u8 k)
/*@*/
{
	return (i32) (((((u64fast) x) << k) - x) >> k);
}

/**@fn tta_postfilter_enc
 * @brief interleave value for coding
 *
 * @param x input value
 *
 * @return interleaved value
 *
 * @note https://en.wikipedia.org/wiki/Golomb_coding#Overview#\
 * Use%20with%20signed%20integers
**/
ALWAYS_INLINE CONST i32
tta_postfilter_enc(register i32 x)
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
 * @note https://en.wikipedia.org/wiki/Golomb_coding#Overview#\
 * Use%20with%20signed%20integers
**/
ALWAYS_INLINE CONST i32
tta_prefilter_dec(register i32 x)
/*@*/
{
	return ((((u32) x) & 0x1u) != 0
		? asr32(++x, (u8) 1u) : asr32(-x, (u8) 1u)
	);
}

//==========================================================================//

/**@fn tta_filter
 * @brief adaptive hybrid filter
 *
 * @param filter[in out] the filter data for the current channel
 * @param round intial sum
 * @param k amount to shift the 'sum' by before add/subtract-ing from 'value'
 * @param value the input value to filter
 * @param mode encode or decode
 *
 * @return
**/
ALWAYS_INLINE i32
tta_filter(
	register struct Filter *const restrict filter, register i32 round,
	register u8 k, register i32 value, const enum TTAMode mode
)
/*@modifies	*filter@*/
{
	register i32 *const restrict a = filter->qm;
	register i32 *const restrict m = filter->dx;
	register i32 *const restrict b = filter->dl;

	register i32 sum = round;
	register uint i;

	// there is a compiler quirk where putting the ==0 branch !first slows
	//   everything down considerably. it adds an branch or two to reduce
	//   code size a bit, but that is just slower. logically, the ==0
	//   branch should be last, because it is the least likely to happen
	//   (but not enough for UNLIKELY; main exception being silence)
	// for-loops SIMD better than unrolled
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
