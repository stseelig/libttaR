//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_dec.c                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>	// true
#include <stddef.h>	// size_t

#include "../bits.h"

#include "state.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_DISABLE_MCH
#undef dest
#undef crc_out
#undef ni32_out
#undef bitcache
#undef codec
static size_t
tta_decode_mch(
	i32 *const dest, const u8 *const, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t, size_t, uint, i32, uint,
	uint
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif

#ifndef LIBTTAr_DISABLE_UNROLLED_1CH
#undef dest
#undef crc_out
#undef ni32_out
#undef bitcache
#undef codec
static size_t
tta_decode_1ch(
	i32 *const dest, const u8 *const, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t, size_t, uint, i32, uint
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif

#ifndef LIBTTAr_DISABLE_UNROLLED_2CH
#undef dest
#undef crc_out
#undef ni32_out
#undef bitcache
#undef codec
static size_t
tta_decode_2ch(
	i32 *const dest, const u8 *const, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t, size_t, uint, i32, uint
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif

//////////////////////////////////////////////////////////////////////////////


// returns 0 on success
int
libttaR_tta_decode(
	i32 *const dest, const u8 *const src,
	size_t dest_len, size_t src_len,
	size_t ni32_target, size_t nbytes_tta_target,
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@partial@*/ struct LibTTAr_CodecState_User *const restrict user,
	enum TTASampleBytes samplebytes, uint nchan
)
/*@modifies	*dest,
		*priv,
		user->is_new_frame,
		user->frame_is_finished,
		user->crc,
		user->ni32,
		user->ni32_total,
		user->nbytes_tta,
		user->nbytes_tta_total
@*/
{
	size_t r;
	const uint predict_k = tta_predict_k(samplebytes);
	const i32 filter_round = tta_filter_round(samplebytes);
	const uint filter_k = tta_filter_k(samplebytes);
	size_t safety_margin = (
		src_len - (TTABUF_SAFETY_MARGIN * nchan * samplebytes)
	);
	if ( nbytes_tta_target < safety_margin ){
		safety_margin = nbytes_tta_target;
	}

	// setup
	state_init(priv, user, nchan);

	// check for bad parameters
	if ( (ni32_target == 0) || (ni32_target > dest_len)
	    ||
	     (ni32_target % nchan != 0)
	){
		return 1;
	}
	if ( nbytes_tta_target > src_len ){
		return 2;
	}
	if ( src_len
	    <=
	     (size_t) (TTABUF_SAFETY_MARGIN * nchan * samplebytes)
	){
		return 3;
	}
	if ( nchan == 0 ){
		return 4;
	}
	if ( (samplebytes == 0) || (samplebytes > TTA_SAMPLEBYTES_MAX) ){
		return 5;
	}

	// decode
	switch ( nchan ){
#ifndef LIBTTAr_DISABLE_UNROLLED_1CH
	case 1u:
		r = tta_decode_1ch(
			dest, src, &user->crc, &user->ni32,
			&priv->bitcache, priv->codec, ni32_target,
			safety_margin, predict_k, filter_round, filter_k
		);
		break;
#endif
#ifndef LIBTTAr_DISABLE_UNROLLED_2CH
	case 2u:
		r = tta_decode_2ch(
			dest, src, &user->crc, &user->ni32,
			&priv->bitcache, priv->codec, ni32_target,
			safety_margin, predict_k, filter_round, filter_k
		);
		break;
#endif
	default:
#ifndef LIBTTAr_DISABLE_MCH
		r = tta_decode_mch(
			dest, src, &user->crc, &user->ni32,
			&priv->bitcache, priv->codec, ni32_target,
			safety_margin, predict_k, filter_round, filter_k,
			nchan
		);
#else
		return -1;
#endif
		break;
	}

	// post-decode
	user->ni32_total += user->ni32;
	if ( user->ni32_total == user->ni32_perframe ){
		user->crc = crc32_end(user->crc);
		user->frame_is_finished = true;
	}
	user->nbytes_tta	= r;
	user->nbytes_tta_total += r;

	return 0;
}

//--------------------------------------------------------------------------//

#ifndef LIBTTAr_DISABLE_MCH
// returns nbytes read from src
static size_t
tta_decode_mch(
	i32 *const dest, const u8 *const src, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t ni32_target,
	size_t safety_margin, uint predict_k, i32 filter_round, uint filter_k,
	uint nchan
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t r = 0;
	u32 crc = *crc_out;
	i32 curr, prev;
	size_t i;
	uint j;

	for ( i = 0; i < ni32_target; i += (size_t) nchan ){
		if ( r > safety_margin ){ break; }
#ifdef LIBTTAr_DISABLE_UNROLLED_1CH
		prev = 0;	// for mono
#endif
		for ( j = 0; true; ++j ){

			// decode
			r = rice_decode(
				(u32 *) &curr, src, r, &codec[j].rice,
				bitcache, &crc
			);

			// filter
			curr = tta_prefilter_dec(curr);
			curr = tta_filter(
				&codec[j].filter, filter_round, filter_k,
				curr, FM_DEC
			);

			// predict
			curr += tta_predict1(codec[j].prev, predict_k);
			codec[j].prev = curr;

			// decorrelate
			if ( j + 1u < nchan ){
				dest[i + j] = curr;
				prev = curr;
			}
			else {	dest[i + j] = (curr += prev / 2);
				while ( j-- != 0 ){
					dest[i + j] = (curr -= dest[i + j]);
				}
				/*@innerbreak@*/ break;
			}
		}
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

#ifndef LIBTTAr_DISABLE_UNROLLED_1CH
static size_t
// returns nbytes read from src
tta_decode_1ch(
	i32 *const dest, const u8 *const src, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t ni32_target,
	size_t safety_margin, uint predict_k, i32 filter_round, uint filter_k
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t r = 0;
	u32 crc = *crc_out;
	i32 curr;
	size_t i;

	for ( i = 0; i < ni32_target; ++i ){
		if ( r > safety_margin ){ break; }

		// decode
		r = rice_decode(
			(u32 *) &curr, src, r, &codec->rice, bitcache, &crc
		);

		// filter
		curr = tta_prefilter_dec(curr);
		curr = tta_filter(
			&codec->filter, filter_round, filter_k, curr, FM_DEC
		);

		// predict
		curr += tta_predict1(codec->prev, predict_k);
		codec->prev = curr;

		// put
		dest[i] = curr;
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

#ifndef LIBTTAr_DISABLE_UNROLLED_2CH
static size_t
// returns nbytes read from src
tta_decode_2ch(
	i32 *const dest, const u8 *const src, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t ni32_target,
	size_t safety_margin, uint predict_k, i32 filter_round, uint filter_k
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t r = 0;
	u32 crc = *crc_out;
	i32 curr, prev;
	size_t i;

	for ( i = 0; i < ni32_target; i += (size_t) 2 ){
		if ( r > safety_margin ){ break; }

	// 0	// decode
		r = rice_decode(
			(u32 *) &curr, src, r, &codec[0].rice, bitcache, &crc
		);

		// filter
		curr = tta_prefilter_dec(curr);
		curr = tta_filter(
			&codec[0].filter, filter_round, filter_k, curr, FM_DEC
		);

		// predict
		curr += tta_predict1(codec[0].prev, predict_k);
		codec[0].prev = curr;

		// save for decorrelation
		prev = curr;

	// 1	// decode
		r = rice_decode(
			(u32 *) &curr, src, r, &codec[1].rice, bitcache, &crc
		);

		// filter
		curr = tta_prefilter_dec(curr);
		curr = tta_filter(
			&codec[1].filter, filter_round, filter_k, curr, FM_DEC
		);

		// predict
		curr += tta_predict1(codec[1].prev, predict_k);
		codec[1].prev = curr;

		// decorrelate
		dest[i + 1u] = (curr += prev / 2);
		dest[i]	     = curr - prev;
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

// EOF ///////////////////////////////////////////////////////////////////////
