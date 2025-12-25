#ifndef H_TTA_CODEC_TTA_DEC_H
#define	H_TTA_CODEC_TTA_DEC_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_dec.h                                                          //
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

#define TTADEC_PARAMS_BASE(x_nchan) \
	const size_t dest_len            = misc->dest_len; \
	const size_t src_len             = misc->src_len; \
	const size_t ni32_target         = misc->ni32_target; \
	const size_t nbytes_tta_target   = misc->nbytes_tta_target; \
	const size_t ni32_perframe       = misc->ni32_perframe; \
	const size_t nbytes_tta_perframe = misc->nbytes_tta_perframe; \
	const enum LibTTAr_SampleBytes samplebytes = misc->samplebytes; \
	const unsigned int             nchan       = (x_nchan); \
	/* * */ \
	enum LibTTAr_DecRetVal retval = LIBTTAr_DRV_OK_AGAIN; \
	size_t nbytes_dec; \
	const bitcnt_dec predict_k    = (bitcnt_dec) ( \
		get_predict_k(samplebytes) \
	); \
	const int32_t    filter_round = get_filter_round(samplebytes); \
	const bitcnt_dec filter_k     = (bitcnt_dec) ( \
		get_filter_k(samplebytes) \
	); \
	const rice24_dec unary_lax_limit = get_unary_lax_limit(samplebytes); \
	const size_t safety_margin    = ( \
		get_safety_margin(samplebytes, nchan) \
	); \
	const size_t read_soft_limit  = (nbytes_tta_target < safety_margin \
		? src_len - safety_margin : nbytes_tta_target \
	);

#ifndef NDEBUG
#define TTADEC_PARAMS(x_nchan) \
	TTADEC_PARAMS_BASE((x_nchan)) \
	const size_t rice_dec_max     = get_rice24_dec_max(samplebytes);
#else
#define TTADEC_PARAMS(x_nchan) \
	TTADEC_PARAMS_BASE((x_nchan))
#endif	/* NDEBUG */

/* ------------------------------------------------------------------------ */

/* @see TTAENC_PARAMCHECKS */

#define TTADEC_PARAMCHECK_0_INVAL_RANGE ( \
	 (dest_len == 0) || (src_len == 0) \
	|| \
	 (ni32_target == 0) || (nbytes_tta_target == 0) \
	|| \
	 (ni32_perframe == 0) || (nbytes_tta_perframe == 0) \
	|| \
	 (nchan == 0) \
)

#define TTADEC_PARAMCHECK_1_INVAL_TRUNC ( \
	ni32_target % nchan != 0 \
)

#define TTADEC_PARAMCHECK_2_INVAL_BOUNDS ( \
	 (ni32_target > dest_len) \
	|| \
	 (ni32_target > ni32_perframe - user->ni32_total) \
)

#define TTADEC_PARAMCHECK_3_INVAL_BOUNDS ( \
	 (src_len < safety_margin) || (src_len < nbytes_tta_target) \
	|| \
	 (nbytes_tta_perframe < user->nbytes_tta_total) \
	|| \
	 ( nbytes_tta_target \
	  > \
	   nbytes_tta_perframe - user->nbytes_tta_total \
	 ) \
)

#define TTADEC_PARAMCHECKS { \
	if ( TTADEC_PARAMCHECK_0_INVAL_RANGE ){ \
		return LIBTTAr_DRV_INVAL_RANGE; \
	} \
	if ( TTADEC_PARAMCHECK_1_INVAL_TRUNC ){ \
		return LIBTTAr_DRV_INVAL_TRUNC; \
	} \
	if ( TTADEC_PARAMCHECK_2_INVAL_BOUNDS ){ \
		return LIBTTAr_DRV_INVAL_BOUNDS; \
	} \
	if ( TTADEC_PARAMCHECK_3_INVAL_BOUNDS ){ \
		return LIBTTAr_DRV_INVAL_BOUNDS; \
	} \
}

/* ------------------------------------------------------------------------ */

#define TTADEC_LOOP_ARGS_BASE \
	dest, src, &user->crc, &user->ni32, &priv->bitcache.dec,  \
	priv->codec, predict_k, filter_round, filter_k, unary_lax_limit,  \
	nchan, ni32_target, read_soft_limit

#ifndef NDEBUG
#define TTADEC_LOOP_ARGS \
		TTADEC_LOOP_ARGS_BASE, rice_dec_max
#else
#define TTADEC_LOOP_ARGS \
		TTADEC_LOOP_ARGS_BASE
#endif	/* NDEBUG */

/* ------------------------------------------------------------------------ */

#define TTADEC_POSTLOOP { \
	int x_overflow_0, x_overflow_1; \
	\
	x_overflow_0 = add_usize_overflow( \
		&user->ni32_total, user->ni32_total, user->ni32 \
	); \
	user->nbytes_tta	= nbytes_dec; \
	x_overflow_1 = add_usize_overflow( \
		&user->nbytes_tta_total, user->nbytes_tta_total, nbytes_dec \
	); \
	if ( (user->ni32_total >= ni32_perframe) \
	    || \
	     (user->nbytes_tta_total >= nbytes_tta_perframe) \
	){ \
		user->crc       = CRC32_FINI(user->crc); \
		if ( (user->ni32_total == ni32_perframe) \
		    && \
		     (user->nbytes_tta_total == nbytes_tta_perframe) \
		){ \
			retval  = LIBTTAr_DRV_OK_DONE; \
		} \
		else {	retval  = LIBTTAr_DRV_FAIL_DECODE; } \
	} \
	user->ncalls_codec     += ( \
		(uint8_t) (user->ncalls_codec != UINT32_MAX) \
	); \
	if ( (x_overflow_0 != 0) || (x_overflow_1 != 0) ){ \
		retval = LIBTTAr_DRV_FAIL_OVERFLOW; \
	} \
}

/* ======================================================================== */

#ifndef NDEBUG
#define TTADEC_RICE(x_chan) { \
	const size_t x_nbytes_old = nbytes_dec; \
	\
	nbytes_dec = rice24_decode( \
		&curr.u, src, nbytes_dec, &codec[(x_chan)].rice.dec, \
		bitcache, &crc, unary_lax_limit \
	); \
	assert(nbytes_dec - x_nbytes_old <= rice_dec_max); \
}
#else
#define TTADEC_RICE(x_chan) { \
	nbytes_dec = rice24_decode( \
		&curr.u, src, nbytes_dec, &codec[(x_chan)].rice.dec, \
		bitcache, &crc, unary_lax_limit \
	); \
}
#endif	/* NDEBUG */

#define TTADEC_FILTER(x_chan) { \
	curr.i  = tta_prefilter_dec(curr.u); \
	curr.i  = tta_filter_dec( \
		&codec[(x_chan)].filter, curr.i, filter_round, \
		(bitcnt) filter_k \
	); \
}

#define TTADEC_PREDICT(x_chan) { \
	curr.i += tta_predict1(codec[(x_chan)].prev, (bitcnt) predict_k); \
	codec[(x_chan)].prev = curr.i; \
}

/* curly braces were messing up the compiler */
#define TTADEC_DECODE(x_chan) \
	TTADEC_RICE(x_chan); \
	TTADEC_FILTER(x_chan); \
	TTADEC_PREDICT(x_chan); \

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_TTA_DEC_H */
