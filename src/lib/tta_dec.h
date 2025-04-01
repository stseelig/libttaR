#ifndef TTA_CODEC_TTA_DEC_H
#define	TTA_CODEC_TTA_DEC_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_dec.h                                                          //
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

#define TTADEC_PARAMS_BASE(Xnchan) \
	const size_t dest_len            = misc->dest_len; \
	const size_t src_len             = misc->src_len; \
	const size_t ni32_target         = misc->ni32_target; \
	const size_t nbytes_tta_target   = misc->nbytes_tta_target; \
	const size_t ni32_perframe       = misc->ni32_perframe; \
	const size_t nbytes_tta_perframe = misc->nbytes_tta_perframe; \
	const enum LibTTAr_SampleBytes samplebytes = misc->samplebytes; \
	UNUSED const uint              nchan       = (Xnchan); \
	\
	enum LibTTAr_DecRetVal retval = LIBTTAr_DRV_AGAIN; \
	size_t nbytes_dec; \
	const bitcnt_dec predict_k    = (bitcnt_dec) ( \
		get_predict_k(samplebytes) \
	); \
	const i32        filter_round = get_filter_round(samplebytes); \
	const bitcnt_dec filter_k     = (bitcnt_dec) ( \
		get_filter_k(samplebytes) \
	); \
	const rice24_dec unary_lax_limit = get_unary_lax_limit(samplebytes); \
	const size_t safety_margin   = ( \
		get_safety_margin(samplebytes, nchan) \
	); \
	const size_t read_soft_limit = (nbytes_tta_target < safety_margin \
		? src_len - safety_margin : nbytes_tta_target \
	);

#ifndef NDEBUG
#define TTADEC_PARAMS(Xnchan) \
	TTADEC_PARAMS_BASE((Xnchan)) \
	const size_t rice_dec_max    = get_rice24_dec_max(samplebytes);
#else
#define TTADEC_PARAMS(Xnchan) \
	TTADEC_PARAMS_BASE((Xnchan))
#endif	// NDEBUG

#define TTADEC_PARAMCHECKS(Xnchan) { \
	/* @see TTAENC_PARAMCHECKS */ \
	if ( (dest_len == 0) || (src_len == 0) \
	    || \
	     (ni32_target == 0) || (nbytes_tta_target == 0) \
	    || \
	     (ni32_perframe == 0) || (nbytes_tta_perframe == 0) \
	    || \
	     ((Xnchan) == 0) \
	){ \
		return LIBTTAr_DRV_INVAL_RANGE; \
	} \
	if ( ((uint) samplebytes == 0) \
	    || \
	     ((uint) samplebytes > LIBTTAr_SAMPLEBYTES_MAX) \
	){ \
		return LIBTTAr_DRV_INVAL_RANGE; \
	} \
	if ( ni32_target % (Xnchan) != 0 ){ \
		return LIBTTAr_DRV_INVAL_TRUNC; \
	} \
	if ( (ni32_target > dest_len) \
	    || \
	     (ni32_target > ni32_perframe - user->ni32_total) \
	){ \
		return LIBTTAr_DRV_INVAL_BOUNDS; \
	} \
	if ( (src_len < safety_margin) || (src_len < nbytes_tta_target) \
	    || \
	     ( nbytes_tta_target \
	      > \
	       nbytes_tta_perframe - user->nbytes_tta_total \
	     ) \
	){ \
		return LIBTTAr_DRV_INVAL_BOUNDS; \
	} \
}

#define TTADEC_LOOP_ARGS_BASE(Xnchan) \
	dest, src, &user->crc, &user->ni32, &priv->bitcache.dec,  \
	priv->codec, predict_k, filter_round, filter_k, unary_lax_limit,  \
	nchan, ni32_target, read_soft_limit

#ifndef NDEBUG
#define TTADEC_LOOP_ARGS(Xnchan) \
		TTADEC_LOOP_ARGS_BASE((Xnchan)), rice_dec_max
#else
#define TTADEC_LOOP_ARGS(Xnchan) \
		TTADEC_LOOP_ARGS_BASE((Xnchan))
#endif	// NDEBUG

#define TTADEC_POSTLOOP { \
	user->ni32_total       += user->ni32; \
	user->nbytes_tta	= nbytes_dec; \
	user->nbytes_tta_total += nbytes_dec; \
	if ( (user->ni32_total == ni32_perframe) \
	    || \
	     (user->nbytes_tta_total > nbytes_tta_perframe) \
	){ \
		user->crc       = CRC32_FINI(user->crc); \
		retval = (user->nbytes_tta_total == nbytes_tta_perframe \
			? LIBTTAr_DRV_DONE : LIBTTAr_DRV_FAIL \
		); \
	} \
	user->ncalls_codec     += (u8) (user->ncalls_codec != UINT32_MAX); \
}

//==========================================================================//

#ifndef NDEBUG
#define TTADEC_DECODE(Xchan) { \
	size_t Xnbytes_old = nbytes_dec; \
	nbytes_dec = rice24_decode( \
		&curr.u, src, nbytes_dec, &codec[(Xchan)].rice.dec, \
		bitcache, &crc, unary_lax_limit \
	); \
	assert(nbytes_dec - Xnbytes_old <= rice_dec_max); \
}
#else
#define TTADEC_DECODE(Xchan) { \
	nbytes_dec = rice24_decode( \
		&curr.u, src, nbytes_dec, &codec[(Xchan)].rice.dec, \
		bitcache, &crc, unary_lax_limit \
	); \
}
#endif	// NDEBUG

#define TTADEC_FILTER(Xchan) { \
	curr.i  = tta_prefilter_dec(curr.u); \
	curr.i  = tta_filter_dec( \
		&codec[(Xchan)].filter, curr.i, filter_round, \
		(bitcnt) filter_k \
	); \
}

#define TTADEC_PREDICT(Xchan) { \
	curr.i += tta_predict1(codec[(Xchan)].prev, (bitcnt) predict_k); \
	codec[(Xchan)].prev = curr.i; \
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
