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

#include <stddef.h>	// size_t

#include "../bits.h"

#include "rice.h"
#include "state.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_MCH
#undef dest
#undef crc_out
#undef ni32_out
#undef bitcache
#undef codec
static size_t tta_encode_mch(
	/*@out@*/ u8 *dest, const i32 *,
	u32 *restrict crc_out, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache *restrict bitcache, struct Codec *restrict codec,
	size_t, size_t, u8, i32, u8, uint
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
#undef dest
#undef crc_out
#undef ni32_out
#undef bitcache
#undef codec
static size_t tta_encode_1ch(
	/*@out@*/ u8 *dest, const i32 *,
	u32 *restrict crc_out, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache *restrict bitcache, struct Codec *restrict codec,
	size_t, size_t, u8, i32, u8, uint
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
#undef dest
#undef crc_out
#undef ni32_out
#undef bitcache
#undef codec
static size_t tta_encode_2ch(
	/*@out@*/ u8 *dest, const i32 *,
	u32 *restrict crc_out, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache *restrict bitcache, struct Codec *restrict codec,
	size_t, size_t, u8, i32, u8, uint
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

/**@fn libttaR_tta_encode
 * @brief a reentrant TTA encoder
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param dest_len size of the destination buffer
 * @param src_len length of the source buffer
 * @param ni32_target target number of source i32 to encode
 * @param priv[in out] private state struct
 * @param user[in out] user readable state struct
 * @param samplebytes number of bytes per PCM sample
 * @param nchan number of audio channels
 * @param ni32_perframe total number of i32 in a TTA frame
 *
 * @return the state of the encoder
 * @retval LIBTTAr_RET_DONE the frame finished
 * @retval LIBTTAr_RET_AGAIN the frame did not finish
 * @retval >=LIBTTAr_RET_INVAL bad parameter
 * @retval LIBTTAr_RET_MISCONFIG library was misconfigured
 *
 * @pre 'user' initialized with LIBTTAr_CODECSTATE_USER_INIT before 1st call
 *
 * @note read the manpage for more info
 * @note affected by:
 *     LIBTTAr_OPT_DISABLE_UNROLLED_1CH,
 *     LIBTTAr_OPT_DISABLE_UNROLLED_2CH,
 *     LIBTTAr_OPT_DISABLE_MCH
**/
int
libttaR_tta_encode(
	/*@out@*/ u8 *const dest, const i32 *const src,
	const size_t dest_len, const size_t src_len, const size_t ni32_target,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const restrict user,
	const enum TTASampleBytes samplebytes, const uint nchan,
	const size_t ni32_perframe
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	int r = LIBTTAr_RET_AGAIN;
	size_t nbytes_tta_enc;
	const  u8 predict_k    = tta_predict_k(samplebytes);
	const i32 filter_round = tta_filter_round(samplebytes);
	const  u8 filter_k     = tta_filter_k(samplebytes);
	const size_t safety_margin    = (size_t) (
		tta_safety_margin_perchan(samplebytes) * nchan
	);
	const size_t write_soft_limit = dest_len - safety_margin;

	// initial state setup
	// inlining this instead of having it as its own library function is
	//   only slower if ni32_target is small and many calls are made for
	//   a frame
	// faster/smaller binary if this is before the checks
	if ( user->ncalls_codec == 0 ){
		state_priv_init(priv, nchan);
	}

	// check for bad parameters
	// having these checks makes it faster, and the order and different
	//   return values matter. not completely sure why
	if ( (src_len == 0) || (dest_len == 0)
	    ||
	     (ni32_target == 0) || (ni32_perframe == 0)
	    ||
	     (nchan == 0)
	){
		return LIBTTAr_RET_INVAL + 0;
	}
	if ( (ni32_target % nchan != 0)
	    ||
	     (ni32_target > src_len)
	    ||
	     (ni32_target + user->ni32_total > ni32_perframe)
	    ||
	     (dest_len < safety_margin)
	){
		return LIBTTAr_RET_INVAL + 1;
	}
	if ( ((uint) samplebytes == 0)
	    ||
	     ((uint) samplebytes > TTA_SAMPLEBYTES_MAX)
	){
		return LIBTTAr_RET_INVAL + 2;
	}

	// encode
	switch ( nchan ){
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
	case 1u:
		nbytes_tta_enc = tta_encode_1ch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, write_soft_limit, predict_k,
			filter_round, filter_k, nchan
		);
		break;
#endif
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
	case 2u:
		nbytes_tta_enc = tta_encode_2ch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, write_soft_limit, predict_k,
			filter_round, filter_k, nchan
		);
		break;
#endif
	default:
#ifndef LIBTTAr_OPT_DISABLE_MCH
		nbytes_tta_enc = tta_encode_mch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, write_soft_limit, predict_k,
			filter_round, filter_k, nchan
		);
		break;
#else
		return LIBTTAr_RET_MISCONFIG;
#endif

#if defined(LIBTTAr_OPT_DISABLE_UNROLLED_1CH) \
 && defined(LIBTTAr_OPT_DISABLE_UNROLLED_2CH) \
 && defined(LIBTTAr_OPT_DISABLE_MCH)
#error "misconfigured codec functions, all channel counts disabled"
#endif
	}

	// post-encode
	user->ni32_total       += user->ni32;
	if ( user->ni32_total == ni32_perframe ){
		nbytes_tta_enc  = rice_encode_cacheflush(
			dest, nbytes_tta_enc, &priv->bitcache, &user->crc
		);
		user->crc       = crc32_end(user->crc);
		r = LIBTTAr_RET_DONE;
	}
	user->nbytes_tta	= nbytes_tta_enc;
	user->nbytes_tta_total += nbytes_tta_enc;

	user->ncalls_codec     += 1u;
	return r;
}

//--------------------------------------------------------------------------//

#ifndef LIBTTAr_OPT_DISABLE_MCH
/**@fn tta_encode_mch
 * @brief multichannel/general encode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_out[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct array
 * @param ni32_target target number of src i32 to read
 * @param write_soft_limit soft limit on the safe number of TTA bytes to write
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 * @param nchan number of audio channels
 *
 * @return number of bytes written to 'dest'
 *
 * @note affected by LIBTTAr_OPT_DISABLE_UNROLLED_1CH
**/
static size_t
tta_encode_mch(
	/*@out@*/ u8 *const dest, const i32 *const src,
	u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, const size_t ni32_target,
	const size_t write_soft_limit, const u8 predict_k,
	const i32 filter_round, const u8 filter_k, const uint nchan
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
		if ( r > write_soft_limit ){ break; }
#ifdef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
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
				dest, (u32) curr, r, &codec[j].rice, bitcache,
				&crc
			);
		}
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
/**@fn tta_encode_1ch
 * @brief unrolled mono encode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_out[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct
 * @param ni32_target target number of src i32 to read
 * @param write_soft_limit soft limit on the safe number of TTA bytes to write
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 *
 * @return number of bytes written to 'dest'
**/
static size_t
tta_encode_1ch(
	/*@out@*/ u8 *const dest, const i32 *const src,
	u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, const size_t ni32_target,
	const size_t write_soft_limit, const u8 predict_k,
	const i32 filter_round, const u8 filter_k, UNUSED const uint nchan
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
		if ( r > write_soft_limit ){ break; }

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
			dest, (u32) curr, r, &codec->rice, bitcache, &crc
		);
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
/**@fn tta_encode_2ch
 * @brief unrolled stereo encode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_out[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct array
 * @param ni32_target target number of src i32 to read
 * @param write_soft_limit soft limit on the safe number of TTA bytes to write
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 *
 * @return number of bytes written to 'dest'
**/
static size_t
tta_encode_2ch(
	/*@out@*/ u8 *const dest, const i32 *const src,
	u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, const size_t ni32_target,
	const size_t write_soft_limit, const u8 predict_k,
	const i32 filter_round, const u8 filter_k, UNUSED const uint nchan
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
		if ( r > write_soft_limit ){ break; }

	// 0	// correlate
		next = src[i + 1u];
		curr = next - src[i];
		prev = curr;

		// predict
		curr -= tta_predict1(codec[0u].prev, predict_k);
		codec[0u].prev = prev;

		// filter
		curr = tta_filter(
			&codec[0u].filter, filter_round, filter_k, curr,
			TTA_ENC
		);
		curr = tta_postfilter_enc(curr);

		// encode
		r = rice_encode(
			dest, (u32) curr, r, &codec[0u].rice, bitcache, &crc
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
			dest, (u32) curr, r, &codec[1u].rice, bitcache, &crc
		);
	}

	*crc_out  = crc;
	*ni32_out = i;
	return r;
}
#endif

// EOF ///////////////////////////////////////////////////////////////////////
