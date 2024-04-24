//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_enc.c                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stddef.h>	// size_t

#include "../bits.h"

#include "rice.h"
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
tta_encode_mch(
	u8 *const dest, const i32 *const, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t, size_t, u8, i32, u8, uint
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
tta_encode_1ch(
	u8 *const dest, const i32 *const, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t, size_t, u8, i32, u8
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
tta_encode_2ch(
	u8 *const dest, const i32 *const, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t, size_t, u8, i32, u8
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
libttaR_tta_encode(
	u8 *const dest, const i32 *const src,
	size_t dest_len, size_t src_len, size_t ni32_target,
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
	const  u8 predict_k = tta_predict_k(samplebytes);
	const i32 filter_round = tta_filter_round(samplebytes);
	const  u8 filter_k = tta_filter_k(samplebytes);
	const size_t safety_margin = (
		  dest_len - (TTABUF_SAFETY_MARGIN_FAST * nchan * samplebytes)
	);


	// initial state setup
	state_init(priv, user, nchan);

	// check for bad parameters
	// having these checks makes it faster, and the order and different
	//  return values matter. not really sure why
	if ( (ni32_target == 0) || (ni32_target > src_len)
	    ||
	     (ni32_target + user->ni32_total > user->ni32_perframe)
	    ||
	     (ni32_target % nchan != 0)
	){
		return 1;
	}
	if ( dest_len
	    <=
	     (size_t) (TTABUF_SAFETY_MARGIN_FAST * nchan * samplebytes)
	){
		return 2;
	}
	if ( nchan == 0 ){
		return 3;
	}
	if ( (samplebytes == 0) || ((uint) samplebytes > TTA_SAMPLEBYTES_MAX)
	){
		return 4;
	}

	// encode
	switch ( nchan ){
#ifndef LIBTTAr_DISABLE_UNROLLED_1CH
	case 1u:
		r = tta_encode_1ch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, safety_margin, predict_k,
			filter_round, filter_k
		);
		break;
#endif
#ifndef LIBTTAr_DISABLE_UNROLLED_2CH
	case 2u:
		r = tta_encode_2ch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, safety_margin, predict_k,
			filter_round, filter_k
		);
		break;
#endif
	default:
#ifndef LIBTTAr_DISABLE_MCH
		r = tta_encode_mch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, safety_margin, predict_k,
			filter_round, filter_k, nchan
		);
		break;
#else
		return -1;
#endif

#if defined(LIBTTAr_DISABLE_UNROLLED_1CH) \
 && defined(LIBTTAr_DISABLE_UNROLLED_2CH) \
 && defined(LIBTTAr_DISABLE_MCH)
#error "misconfigured codec functions, all channel counts disabled"
#endif
	}

	// post-encode
	user->ni32_total += user->ni32;
	if ( user->ni32_total == user->ni32_perframe ){
		r = rice_encode_cacheflush(
			dest, r, &priv->bitcache.cache,
			&priv->bitcache.count, &user->crc
		);
		user->crc = crc32_end(user->crc);
		user->frame_is_finished = true;
	}
	user->nbytes_tta	= r;
	user->nbytes_tta_total += r;

	return 0;
}

//--------------------------------------------------------------------------//

#ifndef LIBTTAr_DISABLE_MCH
// returns nbytes written to dest
static size_t
tta_encode_mch(
	u8 *const dest, const i32 *const src, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t ni32_target,
	size_t safety_margin, u8 predict_k, i32 filter_round, u8 filter_k,
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
		for ( j = 0; j <= nchan - 1u; ++j ){

			// correlate
			curr = src[i + j];
			if ( j < nchan - 1u ){
				curr  = src[i + j + 1u] - curr;
			}
			/*@-usedef@*/	// prev will be defined for non-mono
			else {	curr -= prev / 2; }
			/*@=usedef@*/
			prev = curr;

			// predict
			curr -= tta_predict1(codec[j].prev, predict_k);
			codec[j].prev = prev;

			// filter
			curr = tta_filter(
				&codec[j].filter, filter_round, filter_k,
				curr, TTA_ENC
			);
			curr = tta_postfilter_enc(curr);

			// encode
			r = rice_encode(
				dest, r, (u32) curr, &codec[j].rice,
				bitcache, &crc
			);
		}
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

#ifndef LIBTTAr_DISABLE_UNROLLED_1CH
static size_t
// returns nbytes written to dest
tta_encode_1ch(
	u8 *const dest, const i32 *const src, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t ni32_target,
	size_t safety_margin, u8 predict_k, i32 filter_round, u8 filter_k
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

	for ( i = 0; i < ni32_target; ++i ){
		if ( r > safety_margin ){ break; }

		// get
		curr = src[i];
		prev = curr;

		// predict
		curr -= tta_predict1(codec->prev, predict_k);
		codec->prev = prev;

		// filter
		curr = tta_filter(
			&codec->filter, filter_round, filter_k, curr, TTA_ENC
		);
		curr = tta_postfilter_enc(curr);

		// encode
		r = rice_encode(
			dest, r, (u32) curr, &codec->rice, bitcache, &crc
		);
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

#ifndef LIBTTAr_DISABLE_UNROLLED_2CH
static size_t
// returns nbytes written to dest
tta_encode_2ch(
	u8 *const dest, const i32 *const src, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t ni32_target,
	size_t safety_margin, u8 predict_k, i32 filter_round, u8 filter_k
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
	i32 curr, prev, next;
	size_t i;

	for ( i = 0; i < ni32_target; i += (size_t) 2 ){
		if ( r > safety_margin ){ break; }

	// 0	// correlate
		next = src[i + 1u];
		curr = next - src[i];
		prev = curr;

		// predict
		curr -= tta_predict1(codec[0].prev, predict_k);
		codec[0].prev = prev;

		// filter
		curr = tta_filter(
			&codec[0].filter, filter_round, filter_k, curr, TTA_ENC
		);
		curr = tta_postfilter_enc(curr);

		// encode
		r = rice_encode(
			dest, r, (u32) curr, &codec[0].rice, bitcache, &crc
		);

	// 1	// correlate
		curr = next - (prev / 2);
		prev = curr;

		// predict
		curr -= tta_predict1(codec[1].prev, predict_k);
		codec[1].prev = prev;

		// filter
		curr = tta_filter(
			&codec[1].filter, filter_round, filter_k, curr, TTA_ENC
		);
		curr = tta_postfilter_enc(curr);

		// encode
		r = rice_encode(
			dest, r, (u32) curr, &codec[1].rice, bitcache, &crc
		);
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

// EOF ///////////////////////////////////////////////////////////////////////
