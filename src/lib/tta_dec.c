//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_dec.c                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>	// true
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
tta_decode_mch(
	i32 *const dest, const u8 *const, u32 *const restrict crc_out,
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
tta_decode_1ch(
	i32 *const dest, const u8 *const, u32 *const restrict crc_out,
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
tta_decode_2ch(
	i32 *const dest, const u8 *const, u32 *const restrict crc_out,
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


// returns LIBTTAr_RET_DONE or LIBTTAr_RET_AGAIN on success
int
libttaR_tta_decode(
	i32 *const dest, const u8 *const src,
	size_t dest_len, size_t src_len,
	size_t ni32_target, size_t nbytes_tta_target,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const restrict user,
	enum TTASampleBytes samplebytes, uint nchan, size_t ni32_perframe,
	size_t nbytes_tta_perframe
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	int r = LIBTTAr_RET_AGAIN;
	size_t nbytes_tta_dec;
	const  u8 predict_k    = tta_predict_k(samplebytes);
	const i32 filter_round = tta_filter_round(samplebytes);
	const  u8 filter_k     = tta_filter_k(samplebytes);
	const size_t safety_margin = (size_t) (
		TTABUF_SAFETY_MARGIN_FAST * nchan
	);
	const size_t safety_target = (nbytes_tta_target < safety_margin
		? src_len - safety_margin : nbytes_tta_target
	);

	// initial state setup
	// faster/smaller binary if this is before the checks
	if ( user->ncalls_codec == 0 ){
		state_priv_init(priv, nchan);
	}

	// check for bad parameters
	// having these checks makes it faster, and the order and different
	//   return values matter. not completely sure why
	if ( (src_len == 0) || (dest_len == 0)
	    ||
	     (ni32_target == 0) || (nbytes_tta_target == 0)
	    ||
	     (ni32_perframe == 0) || (nbytes_tta_perframe == 0)
	    ||
	     (nchan == 0)
	){
		return LIBTTAr_RET_INVAL + 0;
	}
	if ( (ni32_target > dest_len)
	    ||
	     (ni32_target + user->ni32_total > ni32_perframe)
	    ||
	     (ni32_target % nchan != 0)
	    ||
	     (nbytes_tta_target > src_len)
	    ||
	     (src_len < safety_margin)
	    ||
	     (	nbytes_tta_target + user->nbytes_tta_total
	       >
	        nbytes_tta_perframe
	     )
	){
		return LIBTTAr_RET_INVAL + 1;
	}
	if ( ((uint) samplebytes == 0)
	    ||
	     ((uint) samplebytes > TTA_SAMPLEBYTES_MAX)
	){
		return LIBTTAr_RET_INVAL + 2;
	}

	// decode
	switch ( nchan ){
#ifndef LIBTTAr_DISABLE_UNROLLED_1CH
	case 1u:
		nbytes_tta_dec = tta_decode_1ch(
			dest, src, &user->crc, &user->ni32,
			&priv->bitcache, priv->codec, ni32_target,
			safety_target, predict_k, filter_round, filter_k
		);
		break;
#endif
#ifndef LIBTTAr_DISABLE_UNROLLED_2CH
	case 2u:
		nbytes_tta_dec = tta_decode_2ch(
			dest, src, &user->crc, &user->ni32,
			&priv->bitcache, priv->codec, ni32_target,
			safety_target, predict_k, filter_round, filter_k
		);
		break;
#endif
	default:
#ifndef LIBTTAr_DISABLE_MCH
		nbytes_tta_dec = tta_decode_mch(
			dest, src, &user->crc, &user->ni32,
			&priv->bitcache, priv->codec, ni32_target,
			safety_target, predict_k, filter_round, filter_k,
			nchan
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

	// post-decode
	user->ni32_total       += user->ni32;
	user->nbytes_tta	= nbytes_tta_dec;
	user->nbytes_tta_total += nbytes_tta_dec;
	if ( (user->ni32_total == ni32_perframe)
	    ||
	     (user->nbytes_tta_total > nbytes_tta_perframe)
	){
		user->crc       = crc32_end(user->crc);
		if ( user->nbytes_tta_total != nbytes_tta_perframe ){
			r = LIBTTAr_RET_DECFAIL;
		}
		else {	r = LIBTTAr_RET_DONE; }
	}

	user->ncalls_codec     += 1u;
	return r;
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
	size_t safety_target, u8 predict_k, i32 filter_round, u8 filter_k,
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
		if ( r > safety_target ){ break; }
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
				curr, TTA_DEC
			);

			// predict
			curr += tta_predict1(codec[j].prev, predict_k);
			codec[j].prev = curr;

			// decorrelate
			if ( j + 1u < nchan ){
				dest[i + j] = curr;
				prev = curr;
			}
			/*@-usedef@*/	// prev will be defined for non-mono
			else {	dest[i + j] = (curr += prev / 2);
			/*@=usedef@*/
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
// returns nbytes read from src
static size_t
tta_decode_1ch(
	i32 *const dest, const u8 *const src, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t ni32_target,
	size_t safety_target, u8 predict_k, i32 filter_round, u8 filter_k
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
		if ( r > safety_target ){ break; }

		// decode
		r = rice_decode(
			(u32 *) &curr, src, r, &codec->rice, bitcache, &crc
		);

		// filter
		curr = tta_prefilter_dec(curr);
		curr = tta_filter(
			&codec->filter, filter_round, filter_k, curr, TTA_DEC
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
// returns nbytes read from src
static size_t
tta_decode_2ch(
	i32 *const dest, const u8 *const src, u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, size_t ni32_target,
	size_t safety_target, u8 predict_k, i32 filter_round, u8 filter_k
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

	for ( i = 0; i < ni32_target; i += (size_t) 2u ){
		if ( r > safety_target ){ break; }

	// 0	// decode
		r = rice_decode(
			(u32 *) &curr, src, r, &codec[0].rice, bitcache, &crc
		);

		// filter
		curr = tta_prefilter_dec(curr);
		curr = tta_filter(
			&codec[0].filter, filter_round, filter_k, curr,
			TTA_DEC
		);

		// predict
		curr += tta_predict1(codec[0].prev, predict_k);
		codec[0].prev = curr;

		// save for decorrelation
		prev = curr;

	// 1	// decode
		r = rice_decode(
			(u32 *) &curr, src, r, &codec[1u].rice, bitcache, &crc
		);

		// filter
		curr = tta_prefilter_dec(curr);
		curr = tta_filter(
			&codec[1u].filter, filter_round, filter_k, curr,
			TTA_DEC
		);

		// predict
		curr += tta_predict1(codec[1u].prev, predict_k);
		codec[1u].prev = curr;

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
