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

// returns LIBTTAr_RET_OK (0) on success
int
libttaR_tta_encode(
	u8 *const dest, const i32 *const src,
	size_t dest_len, size_t src_len, size_t ni32_target,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const restrict user,
	enum TTASampleBytes samplebytes, uint nchan, size_t ni32_perframe
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	size_t nbytes_tta_enc;
	const  u8 predict_k    = tta_predict_k(samplebytes);
	const i32 filter_round = tta_filter_round(samplebytes);
	const  u8 filter_k     = tta_filter_k(samplebytes);
	const size_t safety_margin = (
		dest_len - (TTABUF_SAFETY_MARGIN_FAST * nchan * samplebytes)
	);

	// initial state setup
	if ( user->ncalls_codec == 0 ){
		state_priv_init(priv, nchan);
	}

	// check for bad parameters
	// having these checks makes it faster, and the order and different
	//  return values matter. not completely sure why
	if ( (src_len == 0) || (dest_len == 0)
	    ||
	     (ni32_target == 0) || (ni32_perframe == 0)
	){
		return LIBTTAr_RET_INVAL_BOUNDS;
	}
	if ( (ni32_target > src_len)
	    ||
	     (ni32_target + user->ni32_total > ni32_perframe)
	    ||
	     (ni32_target % nchan != 0)
	    ||
	     (	dest_len
	       <=
	        (size_t) (TTABUF_SAFETY_MARGIN_FAST * nchan * samplebytes)
	     )
	){
		return LIBTTAr_RET_INVAL_BOUNDS;
	}
	if ( (nchan == 0) || (((uint) samplebytes) == 0)
	    ||
	     (((uint) samplebytes) > ((uint) TTA_SAMPLEBYTES_MAX))
	){
		return LIBTTAr_RET_INVAL_DIMEN;
	}

	// encode
	switch ( nchan ){
#ifndef LIBTTAr_DISABLE_UNROLLED_1CH
	case 1u:
		nbytes_tta_enc = tta_encode_1ch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, safety_margin, predict_k,
			filter_round, filter_k
		);
		break;
#endif
#ifndef LIBTTAr_DISABLE_UNROLLED_2CH
	case 2u:
		nbytes_tta_enc = tta_encode_2ch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, safety_margin, predict_k,
			filter_round, filter_k
		);
		break;
#endif
	default:
#ifndef LIBTTAr_DISABLE_MCH
		nbytes_tta_enc = tta_encode_mch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, safety_margin, predict_k,
			filter_round, filter_k, nchan
		);
		break;
#else
		return LIBTTAr_RET_MISCONFIG;
#endif

#if defined(LIBTTAr_DISABLE_UNROLLED_1CH) \
 && defined(LIBTTAr_DISABLE_UNROLLED_2CH) \
 && defined(LIBTTAr_DISABLE_MCH)
#error "misconfigured codec functions, all channel counts disabled"
#endif
	}

	// post-encode
	user->ni32_total += user->ni32;
	if ( user->ni32_total == ni32_perframe ){
		nbytes_tta_enc = rice_encode_cacheflush(
			dest, nbytes_tta_enc, &priv->bitcache.cache,
			&priv->bitcache.count, &user->crc
		);
		user->ncalls_codec  = 0;
		user->crc           = crc32_end(user->crc);
	}
	else {	user->ncalls_codec += (size_t) 1u; }
	user->nbytes_tta	= nbytes_tta_enc;
	user->nbytes_tta_total += nbytes_tta_enc;

	return LIBTTAr_RET_OK;
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
// returns nbytes written to dest
static size_t
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
// returns nbytes written to dest
static size_t
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

	for ( i = 0; i < ni32_target; i += (size_t) 2u ){
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
			&codec[0].filter, filter_round, filter_k, curr,
			TTA_ENC
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
		curr -= tta_predict1(codec[1u].prev, predict_k);
		codec[1u].prev = prev;

		// filter
		curr = tta_filter(
			&codec[1u].filter, filter_round, filter_k, curr,
			TTA_ENC
		);
		curr = tta_postfilter_enc(curr);

		// encode
		r = rice_encode(
			dest, r, (u32) curr, &codec[1u].rice, bitcache, &crc
		);
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

// EOF ///////////////////////////////////////////////////////////////////////
