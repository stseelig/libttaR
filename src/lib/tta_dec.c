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

#include <assert.h>
#include <stdbool.h>	// true
#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"
#include "rice.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_MCH
#undef dest
#undef crc_out
#undef ni32_out
#undef bitcache
#undef codec
static size_t tta_decode_mch(
	/*@out@*/ i32 *const dest, const u8 *,
	u32 *restrict crc_out, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache *restrict bitcache, struct Codec *restrict codec,
	size_t, size_t, bitcnt, i32, bitcnt, rice24, uint
#ifndef NDEBUG
	, size_t
#endif
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
static size_t tta_decode_1ch(
	/*@out@*/ i32 *const dest, const u8 *,
	u32 *restrict crc_out, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache *restrict bitcache, struct Codec *restrict codec,
	size_t, size_t, bitcnt, i32, bitcnt, rice24, uint
#ifndef NDEBUG
	, size_t
#endif
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
static size_t tta_decode_2ch(
	/*@out@*/ i32 *dest, const u8 *,
	u32 *restrict crc_out, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache *restrict bitcache, struct Codec *restrict codec,
	size_t, size_t, bitcnt, i32, bitcnt, rice24, uint
#ifndef NDEBUG
	, size_t
#endif
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

/**@fn libttaR_tta_decode
 * @brief a reentrant TTA decoder
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param dest_len length of the destination buffer
 * @param src_len size of the source buffer
 * @param ni32_target target number of dest i32 to write
 * @param nbytes_tta_target target number of TTA bytes to decode
 * @param priv[in out] private state struct
 * @param user[in out] user readable state struct
 * @param samplebytes number of bytes per PCM sample
 * @param nchan number of audio channels
 * @param ni32_perframe total number of i32 in a TTA frame
 * @param nbytes_tta_perframe number of TTA bytes in the current frame
 *
 * @return the state of the decoder
 * @retval LIBTTAr_RET_DONE the frame finished
 * @retval LIBTTAr_RET_AGAIN the frame did not finish
 * @retval LIBTTAr_RET_DECFAIL the frame failed to decode
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
libttaR_tta_decode(
	/*@out@*/ i32 *const dest, const u8 *const src,
	const size_t dest_len, const size_t src_len,
	const size_t ni32_target, const size_t nbytes_tta_target,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const restrict user,
	const enum TTASampleBytes samplebytes, const uint nchan,
	const size_t ni32_perframe, const size_t nbytes_tta_perframe
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	int r = LIBTTAr_RET_AGAIN;
	size_t nbytes_dec;
	const bitcnt predict_k    = get_predict_k(samplebytes);
	const i32    filter_round = get_filter_round(samplebytes);
	const bitcnt filter_k     = get_filter_k(samplebytes);
	const rice24 unary_lax_limit = get_unary_lax_limit(samplebytes);
	const size_t safety_margin   = get_safety_margin(samplebytes, nchan);
	const size_t read_soft_limit = (nbytes_tta_target < safety_margin
		? src_len - safety_margin : nbytes_tta_target
	);
#ifndef NDEBUG
	const size_t rice_dec_max    = get_rice_dec_max(samplebytes);
#endif
	// initial state setup
	// see libttaR_tta_encode
	if ( user->ncalls_codec == 0 ){
		state_priv_init(priv, nchan);
	}

	// check for bad parameters
	// see libttaR_tta_encode
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
	     (src_len < safety_margin)
	    ||
	     (src_len < nbytes_tta_target)
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
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
	case 1u:
		nbytes_dec = tta_decode_1ch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, read_soft_limit, predict_k,
			filter_round, filter_k, unary_lax_limit, nchan
#ifndef NDEBUG
			, rice_dec_max
#endif
		);
		break;
#endif
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
	case 2u:
		nbytes_dec = tta_decode_2ch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, read_soft_limit, predict_k,
			filter_round, filter_k, unary_lax_limit, nchan
#ifndef NDEBUG
			, rice_dec_max
#endif
		);
		break;
#endif
	default:
#ifndef LIBTTAr_OPT_DISABLE_MCH
		nbytes_dec = tta_decode_mch(
			dest, src, &user->crc, &user->ni32, &priv->bitcache,
			priv->codec, ni32_target, read_soft_limit, predict_k,
			filter_round, filter_k, unary_lax_limit, nchan
#ifndef NDEBUG
			, rice_dec_max
#endif
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

	// post-decode
	user->ni32_total       += user->ni32;
	user->nbytes_tta	= nbytes_dec;
	user->nbytes_tta_total += nbytes_dec;
	if ( (user->ni32_total == ni32_perframe)
	    ||
	     (user->nbytes_tta_total > nbytes_tta_perframe)
	){
		user->crc       = crc32_end(user->crc);
		r = (user->nbytes_tta_total == nbytes_tta_perframe
			? LIBTTAr_RET_DONE : LIBTTAr_RET_DECFAIL
		);
	}
	user->ncalls_codec     += 1u;
	return r;
}

//--------------------------------------------------------------------------//

#ifndef LIBTTAr_OPT_DISABLE_MCH
/**@fn tta_decode_mch
 * @brief multichannel/general decode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_out[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct array
 * @param ni32_target target number of dest i32 to write
 * @param read_soft_limit soft limit on the safe number of TTA bytes to read
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 * @param unary_lax_limit limit for the unary code
 * @param nchan number of audio channels
 *
 * @return number of bytes read from 'src'
 *
 * @note affected by LIBTTAr_OPT_DISABLE_UNROLLED_1CH
**/
static size_t
tta_decode_mch(
	/*@out@*/ i32 *const dest, const u8 *const src,
	u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, const size_t ni32_target,
	const size_t read_soft_limit, const bitcnt predict_k,
	const i32 filter_round, const bitcnt filter_k,
	const rice24 unary_lax_limit, const uint nchan
#ifndef NDEBUG
	, const size_t rice_dec_max
#endif
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t nbytes_dec = 0;
	u32 crc = *crc_out;
	i32 curr, prev;
	size_t i;
	uint j;
#ifndef NDEBUG
	size_t nbytes_old;
#endif
	for ( i = 0; i < ni32_target; i += nchan ){
		if ( nbytes_dec > read_soft_limit ){ break; }
#ifdef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
		prev = 0;
#endif
		for ( j = 0; true; ++j ){

			// decode
#ifndef NDEBUG
			nbytes_old = nbytes_dec;
#endif
			nbytes_dec = rice_decode(
				(u32 *) &curr, src, nbytes_dec,
				&codec[j].rice, bitcache, &crc,
				unary_lax_limit
			);
			assert(nbytes_dec - nbytes_old <= rice_dec_max);

			// filter
			curr = tta_prefilter_dec(curr);
			curr = tta_filter_dec(
				&codec[j].filter, curr, filter_round, filter_k
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
	return nbytes_dec;
}
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
/**@fn tta_decode_1ch
 * @brief unrolled mono decode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_out[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct
 * @param ni32_target target number of dest i32 to write
 * @param read_soft_limit soft limit on the safe number of TTA bytes to read
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 * @param unary_lax_limit limit for the unary code
 * @param nchan unused
 *
 * @return number of bytes read from 'src'
**/
static size_t
tta_decode_1ch(
	/*@out@*/ i32 *const dest, const u8 *const src,
	u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, const size_t ni32_target,
	const size_t read_soft_limit, const bitcnt predict_k,
	const i32 filter_round, const bitcnt filter_k,
	const rice24 unary_lax_limit, UNUSED const uint nchan
#ifndef NDEBUG
	, const size_t rice_dec_max
#endif
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t nbytes_dec = 0;
	u32 crc = *crc_out;
	i32 curr;
	size_t i;
#ifndef NDEBUG
	size_t nbytes_old;
#endif
	for ( i = 0; i < ni32_target; ++i ){
		if ( nbytes_dec > read_soft_limit ){ break; }

		// decode
#ifndef NDEBUG
		nbytes_old = nbytes_dec;
#endif
		nbytes_dec = rice_decode(
			(u32 *) &curr, src, nbytes_dec, &codec->rice,
			bitcache, &crc, unary_lax_limit
		);
		assert(nbytes_dec - nbytes_old <= rice_dec_max);

		// filter
		curr = tta_prefilter_dec(curr);
		curr = tta_filter_dec(
			&codec->filter, curr, filter_round, filter_k
		);

		// predict
		curr += tta_predict1(codec->prev, predict_k);
		codec->prev = curr;

		// put
		dest[i] = curr;
	}
	*crc_out  = crc;
	*ni32_out = i;
	return nbytes_dec;
}
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
/**@fn tta_decode_2ch
 * @brief unrolled stereo decode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_out[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct array
 * @param ni32_target target number of dest i32 to write
 * @param read_soft_limit soft limit on the safe number of TTA bytes to read
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 * @param unary_lax_limit limit for the unary code
 * @param nchan unused
 *
 * @return number of bytes read from 'src'
**/
static size_t
tta_decode_2ch(
	/*@out@*/ i32 *const dest, const u8 *const src,
	u32 *const restrict crc_out,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache *const restrict bitcache,
	struct Codec *const restrict codec, const size_t ni32_target,
	const size_t read_soft_limit, const bitcnt predict_k,
	const i32 filter_round, const bitcnt filter_k,
	const rice24 unary_lax_limit, UNUSED const uint nchan
#ifndef NDEBUG
	, const size_t rice_dec_max
#endif
)
/*@modifies	*dest,
		*crc_out,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t nbytes_dec = 0;
	u32 crc = *crc_out;
	i32 curr, prev;
	size_t i;
#ifndef NDEBUG
	size_t nbytes_old;
#endif
	for ( i = 0; i < ni32_target; i += (size_t) 2u ){
		if ( nbytes_dec > read_soft_limit ){ break; }

	// 0	// decode
#ifndef NDEBUG
		nbytes_old = nbytes_dec;
#endif
		nbytes_dec = rice_decode(
			(u32 *) &curr, src, nbytes_dec, &codec[0u].rice,
			bitcache, &crc, unary_lax_limit
		);
		assert(nbytes_dec - nbytes_old <= rice_dec_max);

		// filter
		curr = tta_prefilter_dec(curr);
		curr = tta_filter_dec(
			&codec[0u].filter, curr, filter_round, filter_k
		);

		// predict
		curr += tta_predict1(codec[0u].prev, predict_k);
		codec[0u].prev = curr;

		// save for decorrelation
		prev = curr;

	// 1	// decode
#ifndef NDEBUG
		nbytes_old = nbytes_dec;
#endif
		nbytes_dec = rice_decode(
			(u32 *) &curr, src, nbytes_dec, &codec[1u].rice,
			bitcache, &crc, unary_lax_limit
		);
		assert(nbytes_dec - nbytes_old <= rice_dec_max);

		// filter
		curr = tta_prefilter_dec(curr);
		curr = tta_filter_dec(
			&codec[1u].filter, curr, filter_round, filter_k
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
	return nbytes_dec;
}
#endif

// EOF ///////////////////////////////////////////////////////////////////////
