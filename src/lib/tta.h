#ifndef TTA_CODEC_TTA_H
#define TTA_CODEC_TTA_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta.h                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "string.h"	// memmove

#include "../bits.h"

#include "rice.h"	// struct Rice

//////////////////////////////////////////////////////////////////////////////

#ifndef xENUM_TTASAMPLEBYTES
#define xENUM_TTASAMPLEBYTES
enum TTASampleBytes {
	TTASAMPLEBYTES_1 = 1u,
	TTASAMPLEBYTES_2 = 2u,
	TTASAMPLEBYTES_3 = 3u
};
#define TTA_SAMPLEBYTES_MAX	((enum TTASampleBytes) 3u)
#define TTA_SAMPLEBITS_MAX	24u
#endif

#define TTABUF_SAFETY_MARGIN	((size_t) 1024)

//////////////////////////////////////////////////////////////////////////////

enum FilterMode {
	FM_ENC,
	FM_DEC
};

//////////////////////////////////////////////////////////////////////////////

struct Filter {
	u8	k;
	i16	round;
	i32	error;
	i32	qm[8];
	i32	dl[9];
	i32	dx[9];
};

struct Codec {
	struct Filter	filter;
	struct Rice	rice;
	i32		prev;
};

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE u16 shift16_bit(register u8) /*@*/;

//--------------------------------------------------------------------------//

#undef codec
INLINE void codec_init(
	register struct Codec *const restrict codec , register uint,
	register enum TTASampleBytes
)
/*@modifies	*codec*/
;

#undef filter
INLINE void filter_init(
	register struct Filter *const restrict filter,
	register enum TTASampleBytes
)
/*@modifies	*filter@*/
;

//--------------------------------------------------------------------------//

ALWAYS_INLINE uint tta_predict_k(register enum TTASampleBytes) /*@*/;
ALWAYS_INLINE i32 tta_predict1(register i32, register uint) /*@*/;
ALWAYS_INLINE i32 tta_postfilter_enc(register i32) /*@*/;
ALWAYS_INLINE i32 tta_prefilter_dec(register i32) /*@*/;

//--------------------------------------------------------------------------//

#undef filter
ALWAYS_INLINE i32 tta_filter(
	register struct Filter *const restrict filter, register i32,
	const enum FilterMode
)
/*@modifies	*filter@*/
;

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE u16 shift16_bit(register u8 k)
/*@*/
{
	return (u16) (0x1u << k);
}

//==========================================================================//

INLINE void
codec_init(
	register struct Codec *const restrict codec, register uint nchan,
	register enum TTASampleBytes samplesize
)
/*@modifies	*codec*/
{
	register uint i;
	for ( i = 0; i < nchan; ++i ){
		filter_init(&codec[i].filter, samplesize);
		rice_init(&codec[i].rice, (u8) 10u, (u8) 10u);
		codec[i].prev = 0;
	}
	return;
}

//--------------------------------------------------------------------------//

INLINE void
filter_init(
	register struct Filter *const restrict filter,
	register enum TTASampleBytes samplesize
)
/*@modifies	*filter@*/
{
	const u8 init_shifts[] = { (u8) 10u, (u8) 9u, (u8) 10u };
	register const  u8 k = init_shifts[samplesize - 1u];
	register const i16 round = (i16) shift16_bit(k - 1u);

	(void) memset(filter, 0x00, sizeof *filter);
	filter->k = k;
	filter->round = round;
	return;
}

//==========================================================================//

// returns 0 on failure
ALWAYS_INLINE uint
tta_predict_k(register enum TTASampleBytes samplebytes)
/*@*/
{
	uint r = 0;

	switch ( samplebytes ){
	case 1u:
		r = 4u;
		break;
	case 2u:
	case 3u:
		r = 5u;
		break;
	}
	return r;
}

ALWAYS_INLINE i32
tta_predict1(register i32 x, register uint k)
/*@*/
{
	return (i32) (((((u64) x) << k) - x) >> k);
}

ALWAYS_INLINE i32
tta_postfilter_enc(register i32 x)
/*@*/
{
	return (x > 0 ? (asl32( x, 1u) - 1) : (asl32(-x, 1u)));
}

ALWAYS_INLINE i32
tta_prefilter_dec(register i32 x)
/*@*/
{
	return ((((u32) x) & 0x1u) != 0 ? asr32(++x, 1u) : asr32( -x, 1u));
}

//==========================================================================//

ALWAYS_INLINE i32
tta_filter(
	register struct Filter *const restrict filter, register i32 value,
	const enum FilterMode mode
)
/*@modifies	*filter@*/
{
	register i32 sum = filter->round;
	register i32 *const restrict a = filter->qm;
	register i32 *const restrict b = filter->dl;
	register i32 *const restrict m = filter->dx;

	// there is a compiler quirk where putting the ==0 branch !first slows
	//  everything down considerably. it adds an extra if-statement or two
	//  to reduce code size a bit, but that borks the autoSIMDing. the
	//  ==0 branch should be last, because it is the least likely to
	//  happen by several order of magnitude
	if ( filter->error == 0 ){
		sum += a[0] * b[0];
		sum += a[1] * b[1];
		sum += a[2] * b[2];
		sum += a[3] * b[3];
		sum += a[4] * b[4];
		sum += a[5] * b[5];
		sum += a[6] * b[6];
		sum += a[7] * b[7];
	}
	else if ( filter->error < 0 ){
		sum += (a[0] -= m[0]) * b[0];
		sum += (a[1] -= m[1]) * b[1];
		sum += (a[2] -= m[2]) * b[2];
		sum += (a[3] -= m[3]) * b[3];
		sum += (a[4] -= m[4]) * b[4];
		sum += (a[5] -= m[5]) * b[5];
		sum += (a[6] -= m[6]) * b[6];
		sum += (a[7] -= m[7]) * b[7];
	}
	else {	// filter->error > 0
		sum += (a[0] += m[0]) * b[0];
		sum += (a[1] += m[1]) * b[1];
		sum += (a[2] += m[2]) * b[2];
		sum += (a[3] += m[3]) * b[3];
		sum += (a[4] += m[4]) * b[4];
		sum += (a[5] += m[5]) * b[5];
		sum += (a[6] += m[6]) * b[6];
		sum += (a[7] += m[7]) * b[7];
	}

	m[8] = (i32) ((((u32) asr32(b[7], 30u)) | 0x1u) << 2u);
	m[7] = (i32) ((((u32) asr32(b[6], 30u)) | 0x1u) << 1u);
	m[6] = (i32) ((((u32) asr32(b[5], 30u)) | 0x1u) << 1u);
	m[5] = (i32)  (((u32) asr32(b[4], 30u)) | 0x1u);

	switch ( mode ){
	case FM_ENC:
		b[8]		 = value;
		value		-= asr32(sum, (uint) filter->k);
		filter->error	 = value;
		break;
	case FM_DEC:
		filter->error	 = value;
		value		+= asr32(sum, (uint) filter->k);
		b[8]		 = value;
		break;
	}

	b[7] = b[8] - b[7];
	b[6] = b[7] - b[6];
	b[5] = b[6] - b[5];

	(void) memmove(b, &b[1], 8*(sizeof *b));
	(void) memmove(m, &m[1], 8*(sizeof *m));

	return value;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
