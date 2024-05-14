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

#ifndef xENUM_TTASAMPLEBYTES
#define xENUM_TTASAMPLEBYTES
enum TTASampleBytes {
	TTASAMPLEBYTES_1 = 1u,
	TTASAMPLEBYTES_2 = 2u,
	TTASAMPLEBYTES_3 = 3u
};
#define TTA_SAMPLEBYTES_MAX	((unsigned int) TTASAMPLEBYTES_3)
#define TTA_SAMPLEBITS_MAX	((uint) (8u*TTA_SAMPLEBYTES_MAX))
#endif

// unary + binary
#define TTABUF_SAFETY_MARGIN_PER_NCHAN		((size_t) (256u + 256u))
// possible extra unary loop
#define TTABUF_SAFETY_MARGIN_24BIT		((size_t) 256u)
// only needed for encode
#define TTABUF_SAFETY_MARGIN_MAX_CACHEFLUSH	((size_t) 32u)
// rounded up to the nearest power of 2
#define TTABUF_SAFETY_MARGIN_FAST		((size_t) 1024u)

//////////////////////////////////////////////////////////////////////////////

enum TTAMode {
	TTA_ENC,
	TTA_DEC
};

//////////////////////////////////////////////////////////////////////////////

struct Filter {
	i32	error;
	i32	qm[8u];
	i32	dl[9u];
	i32	dx[9u];
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

ALWAYS_INLINE u8 tta_predict_k(register enum TTASampleBytes) /*@*/;
ALWAYS_INLINE i32 tta_filter_round(register enum TTASampleBytes) /*@*/;
ALWAYS_INLINE u8 tta_filter_k(register enum TTASampleBytes) /*@*/;

//--------------------------------------------------------------------------//

ALWAYS_INLINE i32 tta_predict1(register i32, register u8) /*@*/;
ALWAYS_INLINE i32 tta_postfilter_enc(register i32) /*@*/;
ALWAYS_INLINE i32 tta_prefilter_dec(register i32) /*@*/;

//--------------------------------------------------------------------------//

#undef filter
ALWAYS_INLINE i32 tta_filter(
	register struct Filter *const restrict filter, register i32,
	register u8, register i32, const enum TTAMode
)
/*@modifies	*filter@*/
;

//////////////////////////////////////////////////////////////////////////////

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

ALWAYS_INLINE u8
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

ALWAYS_INLINE i32
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

ALWAYS_INLINE u8
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

ALWAYS_INLINE i32
tta_predict1(register i32 x, register u8 k)
/*@*/
{
	return (i32) (((((u64fast) x) << k) - x) >> k);
}

ALWAYS_INLINE i32
tta_postfilter_enc(register i32 x)
/*@*/
{
	return (x > 0 ? asl32(x, (u8) 1u) - 1 : asl32(-x, (u8) 1u));
}

ALWAYS_INLINE i32
tta_prefilter_dec(register i32 x)
/*@*/
{
	return ((((u32) x) & 0x1u) != 0
		? asr32(++x, (u8) 1u)
		: asr32( -x, (u8) 1u)
	);
}

//==========================================================================//

ALWAYS_INLINE i32
tta_filter(
	register struct Filter *const restrict filter,
	register i32 round, register u8 k, register i32 value,
	const enum TTAMode mode
)
/*@modifies	*filter@*/
{
	register i32 *const restrict a = filter->qm;
	register i32 *const restrict b = filter->dl;
	register i32 *const restrict m = filter->dx;

	register i32 sum = round;

	// there is a compiler quirk where putting the ==0 branch !first slows
	//  everything down considerably. it adds an extra if-statement or two
	//  to reduce code size a bit, but that borks the assembler. the ==0
	//  branch should be last, because it is the least likely to happen
	//  (but not enough for UNLIKELY; main exception being silence)
	if ( filter->error == 0 ){
		sum += a[0u] * b[0u];
		sum += a[1u] * b[1u];
		sum += a[2u] * b[2u];
		sum += a[3u] * b[3u];
		sum += a[4u] * b[4u];
		sum += a[5u] * b[5u];
		sum += a[6u] * b[6u];
		sum += a[7u] * b[7u];
	}
	else if ( filter->error < 0 ){
		sum += (a[0u] -= m[0u]) * b[0u];
		sum += (a[1u] -= m[1u]) * b[1u];
		sum += (a[2u] -= m[2u]) * b[2u];
		sum += (a[3u] -= m[3u]) * b[3u];
		sum += (a[4u] -= m[4u]) * b[4u];
		sum += (a[5u] -= m[5u]) * b[5u];
		sum += (a[6u] -= m[6u]) * b[6u];
		sum += (a[7u] -= m[7u]) * b[7u];
	}
	else {	// filter->error > 0
		sum += (a[0u] += m[0u]) * b[0u];
		sum += (a[1u] += m[1u]) * b[1u];
		sum += (a[2u] += m[2u]) * b[2u];
		sum += (a[3u] += m[3u]) * b[3u];
		sum += (a[4u] += m[4u]) * b[4u];
		sum += (a[5u] += m[5u]) * b[5u];
		sum += (a[6u] += m[6u]) * b[6u];
		sum += (a[7u] += m[7u]) * b[7u];
	}

	m[8u] = (i32) ((((u32) asr32(b[7u], (u8) 30u)) | 0x1u) << 2u);
	m[7u] = (i32) ((((u32) asr32(b[6u], (u8) 30u)) | 0x1u) << 1u);
	m[6u] = (i32) ((((u32) asr32(b[5u], (u8) 30u)) | 0x1u) << 1u);
	m[5u] = (i32) ((((u32) asr32(b[4u], (u8) 30u)) | 0x1u) << 0u);

	switch ( mode ){
	case TTA_ENC:
		b[8u]		 = value;
		value		-= asr32(sum, k);
		filter->error	 = value;
		break;
	case TTA_DEC:
		filter->error	 = value;
		value		+= asr32(sum, k);
		b[8u]		 = value;
		break;
	}

	b[7u] = b[8u] - b[7u];
	b[6u] = b[7u] - b[6u];
	b[5u] = b[6u] - b[5u];

	MEMMOVE(b, &b[1u], (size_t) (8u*(sizeof *b)));
	MEMMOVE(m, &m[1u], (size_t) (8u*(sizeof *m)));

	return value;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
