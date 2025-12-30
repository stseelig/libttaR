#ifndef H_TTA_CODEC_TTA_ENC_H
#define	H_TTA_CODEC_TTA_ENC_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_enc.h                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "./common.h"
#include "./filter.h"
#include "./overflow.h"
#include "./rice24.h"
#include "./tta.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

#define TTAENC_PARAMS_BASE(x_nchan) \
	const size_t dest_len      = misc->dest_len; \
	const size_t src_len       = misc->src_len; \
	const size_t ni32_target   = misc->ni32_target; \
	const size_t ni32_perframe = misc->ni32_perframe; \
	const enum LibTTAr_SampleBytes samplebytes = misc->samplebytes; \
	const unsigned int             nchan       = (x_nchan); \
	/* * */ \
	enum LibTTAr_EncRetVal retval = LIBTTAr_ERV_OK_AGAIN; \
	size_t nbytes_enc; \
	const bitcnt_enc predict_k    = (bitcnt_enc) ( \
		get_predict_k(samplebytes) \
	); \
	const int32_t    filter_round = get_filter_round(samplebytes); \
	const bitcnt_enc filter_k     = (bitcnt_enc) ( \
		get_filter_k(samplebytes) \
	); \
	const size_t safety_margin    = ( \
		get_safety_margin(samplebytes, nchan) \
	); \
	const size_t write_soft_limit = dest_len - safety_margin;

#ifndef NDEBUG
#define TTAENC_PARAMS(x_nchan) \
	TTAENC_PARAMS_BASE((x_nchan)) \
	const size_t rice_enc_max     = get_rice24_enc_max(samplebytes);
#else
#define TTAENC_PARAMS(x_nchan) \
	TTAENC_PARAMS_BASE((x_nchan))
#endif	/* NDEBUG */

/* ------------------------------------------------------------------------ */

/* having these checks makes it faster, and the order and different return
     values matter. not completely sure why
*/

#define TTAENC_PARAMCHECK_0_INVAL_RANGE ( \
	 (dest_len == 0) || (src_len == 0) \
	|| \
	 (ni32_target == 0) || (ni32_perframe == 0) \
	|| \
	 (nchan == 0) || (safety_margin == 0) \
)

#define TTAENC_PARAMCHECK_1_INVAL_TRUNC ( \
	ni32_target % nchan != 0 \
)

#define TTAENC_PARAMCHECK_2_INVAL_BOUNDS ( \
	dest_len < safety_margin \
)

#define TTAENC_PARAMCHECK_3_INVAL_BOUNDS ( \
	 (ni32_target > src_len) \
	|| \
	 (ni32_perframe < user->ni32_total) \
	|| \
	 (ni32_target > ni32_perframe - user->ni32_total) \
)

#define TTAENC_PARAMCHECKS { \
	if ( TTAENC_PARAMCHECK_0_INVAL_RANGE ){ \
		return LIBTTAr_ERV_INVAL_RANGE; \
	} \
	if ( TTAENC_PARAMCHECK_1_INVAL_TRUNC ){ \
		return LIBTTAr_ERV_INVAL_TRUNC; \
	} \
	if ( TTAENC_PARAMCHECK_2_INVAL_BOUNDS ){ \
		return LIBTTAr_ERV_INVAL_BOUNDS; \
	} \
	if ( TTAENC_PARAMCHECK_3_INVAL_BOUNDS ){ \
		return LIBTTAr_ERV_INVAL_BOUNDS; \
	} \
}

/* ------------------------------------------------------------------------ */

#define TTAENC_LOOP_ARGS_BASE \
	dest, src, &user->crc, &user->ni32, &priv->bitcache.enc, \
	priv->codec, predict_k, filter_round, filter_k, nchan, \
	ni32_target, write_soft_limit

#ifndef NDEBUG
#define TTAENC_LOOP_ARGS \
		TTAENC_LOOP_ARGS_BASE, rice_enc_max
#else
#define TTAENC_LOOP_ARGS \
		TTAENC_LOOP_ARGS_BASE
#endif	/* NDEBUG */

/* ------------------------------------------------------------------------ */

#define TTAENC_POSTLOOP { \
	int x_overflow_0, x_overflow_1; \
	\
	x_overflow_0 = add_usize_overflow( \
		&user->ni32_total, user->ni32_total, user->ni32 \
	); \
	if ( user->ni32_total == ni32_perframe ){ \
		nbytes_enc = rice24_encode_cacheflush( \
			dest, nbytes_enc, &priv->bitcache.enc, &user->crc \
		); \
		user->crc       = CRC32_FINI(user->crc); \
		retval     = LIBTTAr_ERV_OK_DONE; \
	} \
	user->nbytes_tta	= nbytes_enc; \
	x_overflow_1 = add_usize_overflow( \
		&user->nbytes_tta_total, user->nbytes_tta_total, nbytes_enc \
	); \
	user->ncalls_codec     += ( \
		(uint8_t) (user->ncalls_codec != UINT32_MAX) \
	); \
	if ( (x_overflow_0 != 0) || (x_overflow_1 != 0) ){ \
		retval     = LIBTTAr_ERV_FAIL_OVERFLOW; \
	} \
}

/* ======================================================================== */

#define TTAENC_PREDICT(x_chan) { \
	prev    = curr.i; \
	curr.i -= tta_predict1(codec[(x_chan)].prev, (bitcnt) predict_k); \
	codec[(x_chan)].prev = prev; \
}

#define TTAENC_FILTER(x_chan) { \
	curr.i  = tta_filter_enc( \
		&codec[(x_chan)].filter, curr.i, filter_round, \
		(bitcnt) filter_k \
	); \
	curr.u  = tta_postfilter_enc(curr.i); \
}

#ifndef NDEBUG
#define TTAENC_RICE(x_chan) { \
	const size_t x_nbytes_old = nbytes_enc; \
	\
	nbytes_enc = rice24_encode( \
		dest, curr.u, nbytes_enc, &codec[(x_chan)].rice.enc, \
		bitcache, &crc \
	); \
	assert(nbytes_enc - x_nbytes_old <= rice_enc_max); \
}
#else
#define TTAENC_RICE(x_chan) { \
	nbytes_enc = rice24_encode( \
		dest, curr.u, nbytes_enc, &codec[(x_chan)].rice.enc, \
		bitcache, &crc \
	); \
}
#endif	/* NDEBUG */

/* curly braces were messing up the compiler */
#define TTAENC_ENCODE(x_chan) \
	TTAENC_PREDICT(x_chan); \
	TTAENC_FILTER(x_chan); \
	TTAENC_RICE(x_chan);

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_TTA_ENC_H */
