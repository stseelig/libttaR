//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_enc.c                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"
#include "filter.h"
#include "rice24.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_MCH
#undef dest
#undef crc_inout
#undef ni32_out
#undef bitcache
#undef codec
static HOT size_t tta_encode_mch(
	/*@out@*/ u8 *dest, const i32 *,
	u32 *restrict crc_inout, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache_Enc *restrict bitcache, struct Codec *restrict codec,
	size_t, size_t, bitcnt_enc, i32, bitcnt_enc, uint
#ifndef NDEBUG
	, size_t
#endif
)
/*@modifies	*dest,
		*crc_inout,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
#undef dest
#undef crc_inout
#undef ni32_out
#undef bitcache
#undef codec
static HOT size_t tta_encode_1ch(
	/*@out@*/ u8 *dest, const i32 *,
	u32 *restrict crc_inout, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache_Enc *restrict bitcache, struct Codec *restrict codec,
	size_t, size_t, bitcnt_enc, i32, bitcnt_enc, uint
#ifndef NDEBUG
	, size_t
#endif
)
/*@modifies	*dest,
		*crc_inout,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
#undef dest
#undef crc_inout
#undef ni32_out
#undef bitcache
#undef codec
static HOT size_t tta_encode_2ch(
	/*@out@*/ u8 *dest, const i32 *,
	u32 *restrict crc_inout, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache_Enc *restrict bitcache, struct Codec *restrict codec,
	size_t, size_t, bitcnt_enc, i32, bitcnt_enc, uint
#ifndef NDEBUG
	, size_t
#endif
)
/*@modifies	*dest,
		*crc_inout,
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
HOT int
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
	size_t nbytes_enc;
	const bitcnt_enc predict_k    = (bitcnt_enc) (
		get_predict_k(samplebytes)
	);
	const i32        filter_round = get_filter_round(samplebytes);
	const bitcnt_enc filter_k     = (bitcnt_enc) (
		get_filter_k(samplebytes)
	);
	const size_t safety_margin    = get_safety_margin(samplebytes, nchan);
	const size_t write_soft_limit = dest_len - safety_margin;
#ifndef NDEBUG
	const size_t rice_enc_max     = get_rice24_enc_max(samplebytes);
#endif
	// initial state setup
	// inlining this instead of having it as its own library function is
	//   only slower if ni32_target is small and many calls are made for
	//   a frame
	// faster/smaller binary if this is before the checks
	if ( user->ncalls_codec == 0 ){
		state_priv_init_enc(priv, nchan);
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
	     (ni32_target > ni32_perframe - user->ni32_total)
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
	default:
#ifndef LIBTTAr_OPT_DISABLE_MCH
		nbytes_enc = tta_encode_mch(
			dest, src, &user->crc, &user->ni32,
			&priv->bitcache.enc, priv->codec, ni32_target,
			write_soft_limit, predict_k, filter_round, filter_k,
			nchan
#ifndef NDEBUG
			, rice_enc_max
#endif
		);
		break;
#else
		return LIBTTAr_RET_MISCONFIG;
#endif
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
	case 1u:
		nbytes_enc = tta_encode_1ch(
			dest, src, &user->crc, &user->ni32,
			&priv->bitcache.enc, priv->codec, ni32_target,
			write_soft_limit, predict_k, filter_round, filter_k,
			nchan
#ifndef NDEBUG
			, rice_enc_max
#endif
		);
		break;
#endif
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
	case 2u:
		nbytes_enc = tta_encode_2ch(
			dest, src, &user->crc, &user->ni32,
			&priv->bitcache.enc, priv->codec, ni32_target,
			write_soft_limit, predict_k, filter_round, filter_k,
			nchan
#ifndef NDEBUG
			, rice_enc_max
#endif
		);
		break;
#endif
	}
#if defined(LIBTTAr_OPT_DISABLE_UNROLLED_1CH) \
 && defined(LIBTTAr_OPT_DISABLE_UNROLLED_2CH) \
 && defined(LIBTTAr_OPT_DISABLE_MCH)
#error "misconfigured codec functions, all channel counts disabled"
#endif

	// post-encode
	user->ni32_total       += user->ni32;
	if ( user->ni32_total == ni32_perframe ){
		nbytes_enc      = rice24_encode_cacheflush(
			dest, nbytes_enc, &priv->bitcache.enc, &user->crc
		);
		user->crc       = CRC32_FINI(user->crc);
		r = LIBTTAr_RET_DONE;
	}
	user->nbytes_tta	= nbytes_enc;
	user->nbytes_tta_total += nbytes_enc;
	user->ncalls_codec     += (u8) (user->ncalls_codec != UINT32_MAX);
	return r;
}

//--------------------------------------------------------------------------//

#define TTAENC_PREDICT(Xchan) { \
	prev    = curr.i; \
	curr.i -= tta_predict1(codec[Xchan].prev, (bitcnt) predict_k); \
	codec[Xchan].prev = prev; \
}
#define TTAENC_FILTER(Xchan) { \
	curr.i  = tta_filter_enc( \
		&codec[Xchan].filter, curr.i, filter_round, \
		(bitcnt) filter_k \
	); \
	curr.u  = tta_postfilter_enc(curr.i); \
}
#ifndef NDEBUG
#define TTAENC_ENCODE(Xchan) { \
	size_t Xnbytes_old = nbytes_enc; \
	nbytes_enc = rice24_encode( \
		dest, curr.u, nbytes_enc, &codec[Xchan].rice.enc, bitcache, \
		&crc \
	); \
	assert(nbytes_enc - Xnbytes_old <= rice_enc_max); \
}
#else
#define TTAENC_ENCODE(Xchan) { \
	nbytes_enc = rice24_encode( \
		dest, curr.u, nbytes_enc, &codec[Xchan].rice.enc, bitcache, \
		&crc \
	); \
}
#endif	// NDEBUG

#ifndef LIBTTAr_OPT_DISABLE_MCH
/**@fn tta_encode_mch
 * @brief multichannel/general encode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_inout[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct array
 * @param ni32_target target number of src i32 to read
 * @param write_soft_limit soft limit on the safe number of TTA bytes to write
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 * @param nchan number of audio channels
 * @param rice_enc_max debug value for theoretical max unary/binary code size
 *
 * @return number of bytes written to 'dest'
 *
 * @note affected by LIBTTAr_OPT_DISABLE_UNROLLED_1CH
**/
static HOT size_t
tta_encode_mch(
	/*@out@*/ u8 *const dest, const i32 *const src,
	u32 *const restrict crc_inout,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache_Enc *const restrict bitcache,
	struct Codec *const restrict codec, const size_t ni32_target,
	const size_t write_soft_limit, const bitcnt_enc predict_k,
	const i32 filter_round, const bitcnt_enc filter_k, const uint nchan
#ifndef NDEBUG
	, const size_t rice_enc_max
#endif
)
/*@modifies	*dest,
		*crc_inout,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t nbytes_enc = 0;
	crc32_enc crc = (crc32_enc) *crc_inout;
	union { i32 i; u32 u; } curr;
	i32 prev;
	size_t i;
	uint j;

	for ( i = 0; i < ni32_target; i += nchan ){
		if ( nbytes_enc > write_soft_limit ){ break; }
#ifdef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
		prev = 0;
#endif
		for ( j = 0; j <= nchan - 1u; ++j ){

			// correlate
			curr.i  = src[i + j];
			if ( j < nchan - 1u ){
				curr.i  = src[i + j + 1u] - curr.i;
			}
			else {	/*@-usedef@*/	// prev defined for non-mono
				curr.i -= prev / 2;
				/*@=usedef@*/
			}

			TTAENC_PREDICT(j);
			TTAENC_FILTER(j);
			TTAENC_ENCODE(j);
		}
	}
	*crc_inout = (u32) crc;
	*ni32_out  = i;
	return nbytes_enc;
}
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
/**@fn tta_encode_1ch
 * @brief unrolled mono encode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_inout[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct
 * @param ni32_target target number of src i32 to read
 * @param write_soft_limit soft limit on the safe number of TTA bytes to write
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 * @param nchan unused
 * @param rice_enc_max debug value for theoretical max unary/binary code size
 *
 * @return number of bytes written to 'dest'
**/
static HOT size_t
tta_encode_1ch(
	/*@out@*/ u8 *const dest, const i32 *const src,
	u32 *const restrict crc_inout,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache_Enc *const restrict bitcache,
	struct Codec *const restrict codec, const size_t ni32_target,
	const size_t write_soft_limit, const bitcnt_enc predict_k,
	const i32 filter_round, const bitcnt_enc filter_k,
	UNUSED const uint nchan
#ifndef NDEBUG
	, const size_t rice_enc_max
#endif
)
/*@modifies	*dest,
		*crc_inout,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t nbytes_enc = 0;
	crc32_enc crc = (crc32_enc) *crc_inout;
	union { i32 i; u32 u; } curr;
	i32 prev;
	size_t i;

	for ( i = 0; i < ni32_target; ++i ){
		if ( nbytes_enc > write_soft_limit ){ break; }

		curr.i = src[i];
		TTAENC_PREDICT(0);
		TTAENC_FILTER(0);
		TTAENC_ENCODE(0);
	}
	*crc_inout = (u32) crc;
	*ni32_out  = i;
	return nbytes_enc;
}
#endif

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
/**@fn tta_encode_2ch
 * @brief unrolled stereo encode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_inout[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct array
 * @param ni32_target target number of src i32 to read
 * @param write_soft_limit soft limit on the safe number of TTA bytes to write
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 * @param nchan unused
 * @param rice_enc_max debug value for theoretical max unary/binary code size
 *
 * @return number of bytes written to 'dest'
**/
static HOT size_t
tta_encode_2ch(
	/*@out@*/ u8 *const dest, const i32 *const src,
	u32 *const restrict crc_inout,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache_Enc *const restrict bitcache,
	struct Codec *const restrict codec, const size_t ni32_target,
	const size_t write_soft_limit, const bitcnt_enc predict_k,
	const i32 filter_round, const bitcnt_enc filter_k,
	UNUSED const uint nchan
#ifndef NDEBUG
	, const size_t rice_enc_max
#endif
)
/*@modifies	*dest,
		*crc_inout,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t nbytes_enc = 0;
	crc32_enc crc = (crc32_enc) *crc_inout;
	union { i32 i; u32 u; } curr;
	i32 prev, next;
	size_t i;

	for ( i = 0; i < ni32_target; i += (size_t) 2u ){
		if ( nbytes_enc > write_soft_limit ){ break; }
	// 0
		next   = src[i + 1u];
		curr.i = next - src[i + 0u];
		TTAENC_PREDICT(0u);
		TTAENC_FILTER(0u);
		TTAENC_ENCODE(0u);
	// 1
		curr.i = next - (prev / 2);
		TTAENC_PREDICT(1u);
		TTAENC_FILTER(1u);
		TTAENC_ENCODE(1u);
	}
	*crc_inout = (u32) crc;
	*ni32_out  = i;
	return nbytes_enc;
}
#endif

// EOF ///////////////////////////////////////////////////////////////////////
