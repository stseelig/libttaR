#ifndef TTA_CODEC_TTA_ENC_H
#define	TTA_CODEC_TTA_ENC_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_enc.h                                                          //
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

#define TTAENC_PARAMS_BASE(Xnchan) \
	const size_t dest_len      = misc->dest_len; \
	const size_t src_len       = misc->src_len; \
	const size_t ni32_target   = misc->ni32_target; \
	const size_t ni32_perframe = misc->ni32_perframe; \
	const enum LibTTAr_SampleBytes samplebytes = misc->samplebytes; \
	const uint                     nchan       = (Xnchan); \
	\
	enum LibTTAr_EncRetVal retval = LIBTTAr_ERV_AGAIN; \
	size_t nbytes_enc; \
	const bitcnt_enc predict_k    = (bitcnt_enc) ( \
		get_predict_k(samplebytes) \
	); \
	const i32        filter_round = get_filter_round(samplebytes); \
	const bitcnt_enc filter_k     = (bitcnt_enc) ( \
		get_filter_k(samplebytes) \
	); \
	const size_t safety_margin    = ( \
		get_safety_margin(samplebytes, nchan) \
	); \
	const size_t write_soft_limit = dest_len - safety_margin;

#ifndef NDEBUG
#define TTAENC_PARAMS(Xnchan) \
	TTAENC_PARAMS_BASE((Xnchan)) \
	const size_t rice_enc_max     = get_rice24_enc_max(samplebytes);
#else
#define TTAENC_PARAMS(Xnchan) \
	TTAENC_PARAMS_BASE((Xnchan))
#endif	// NDEBUG

//--------------------------------------------------------------------------//

// having these checks makes it faster, and the order and different return
//   values matter. not completely sure why

#define TTAENC_PARAMCHECK_0_INVAL_RANGE ( \
	 (dest_len == 0) || (src_len == 0) \
	|| \
	 (ni32_target == 0) || (ni32_perframe == 0) \
	|| \
	 (nchan == 0) \
)

#define TTAENC_PARAMCHECK_1_INVAL_RANGE ( \
	 ((uint) samplebytes == 0) \
	|| \
	 ((uint) samplebytes > LIBTTAr_SAMPLEBYTES_MAX) \
)

#define TTAENC_PARAMCHECK_2_INVAL_TRUNC ( \
	ni32_target % nchan != 0 \
)

#define TTAENC_PARAMCHECK_3_INVAL_BOUNDS ( \
	dest_len < safety_margin \
)

#define TTAENC_PARAMCHECK_4_INVAL_BOUNDS ( \
	 (ni32_target > src_len) \
	|| \
	 (ni32_target > ni32_perframe - user->ni32_total) \
)

#define TTAENC_PARAMCHECKS { \
	if ( TTAENC_PARAMCHECK_0_INVAL_RANGE ){ \
		assert(! TTAENC_PARAMCHECK_0_INVAL_RANGE); \
		return LIBTTAr_ERV_INVAL_RANGE; \
	} \
	if ( TTAENC_PARAMCHECK_1_INVAL_RANGE ){ \
		assert(! TTAENC_PARAMCHECK_1_INVAL_RANGE); \
		return LIBTTAr_ERV_INVAL_RANGE; \
	} \
	if ( TTAENC_PARAMCHECK_2_INVAL_TRUNC ){ \
		assert(! TTAENC_PARAMCHECK_2_INVAL_TRUNC); \
		return LIBTTAr_ERV_INVAL_TRUNC; \
	} \
	if ( TTAENC_PARAMCHECK_3_INVAL_BOUNDS ){ \
		assert(! TTAENC_PARAMCHECK_3_INVAL_BOUNDS); \
		return LIBTTAr_ERV_INVAL_BOUNDS; \
	} \
	if ( TTAENC_PARAMCHECK_4_INVAL_BOUNDS ){ \
		assert(! TTAENC_PARAMCHECK_4_INVAL_BOUNDS); \
		return LIBTTAr_ERV_INVAL_BOUNDS; \
	} \
}

//--------------------------------------------------------------------------//

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
#endif	// NDEBUG

//--------------------------------------------------------------------------//

#define TTAENC_POSTLOOP { \
	user->ni32_total       += user->ni32; \
	if ( user->ni32_total == ni32_perframe ){ \
		nbytes_enc = rice24_encode_cacheflush( \
			dest, nbytes_enc, &priv->bitcache.enc, &user->crc \
		); \
		user->crc       = CRC32_FINI(user->crc); \
		retval     = LIBTTAr_ERV_DONE; \
	} \
	user->nbytes_tta	= nbytes_enc; \
	user->nbytes_tta_total += nbytes_enc; \
	user->ncalls_codec     += (u8) (user->ncalls_codec != UINT32_MAX); \
}

//==========================================================================//

#define TTAENC_PREDICT(Xchan) { \
	prev    = curr.i; \
	curr.i -= tta_predict1(codec[(Xchan)].prev, (bitcnt) predict_k); \
	codec[(Xchan)].prev = prev; \
}

#define TTAENC_FILTER(Xchan) { \
	curr.i  = tta_filter_enc( \
		&codec[(Xchan)].filter, curr.i, filter_round, \
		(bitcnt) filter_k \
	); \
	curr.u  = tta_postfilter_enc(curr.i); \
}

#ifndef NDEBUG
#define TTAENC_RICE(Xchan) { \
	size_t Xnbytes_old = nbytes_enc; \
	nbytes_enc = rice24_encode( \
		dest, curr.u, nbytes_enc, &codec[(Xchan)].rice.enc, \
		bitcache, &crc \
	); \
	assert(nbytes_enc - Xnbytes_old <= rice_enc_max); \
}
#else
#define TTAENC_RICE(Xchan) { \
	nbytes_enc = rice24_encode( \
		dest, curr.u, nbytes_enc, &codec[(Xchan)].rice.enc, \
		bitcache, &crc \
	); \
}
#endif	// NDEBUG

#define TTAENC_ENCODE(Xchan) \
	TTAENC_PREDICT(Xchan); \
	TTAENC_FILTER(Xchan); \
	TTAENC_RICE(Xchan);


// EOF ///////////////////////////////////////////////////////////////////////
#endif
