//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_enc_c_2ch.c                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"
#include "tta.h"
#include "tta_enc.h"

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
#undef dest
#undef crc_inout
#undef ni32_out
#undef bitcache
#undef codec
ALWAYS_INLINE size_t tta_encode_2ch_loop(
	/*@reldef@*/ u8 *restrict dest, const i32 *restrict,
	u32 *restrict crc_inout, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache_Enc *restrict bitcache, struct Codec *restrict codec,
	bitcnt_enc, i32, bitcnt_enc, uint, size_t, size_t
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
#endif	// LIBTTAr_OPT_DISABLE_UNROLLED_2CH

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
/**@fn tta_encode_2ch
 * @brief stereo encoder
 *
 * @see libttaR_tta_encode()
**/
HIDDEN enum LibTTAr_EncRetVal
tta_encode_2ch(
	/*@reldef@*/ u8 *const restrict dest,
	/*@in@*/ const i32 *const restrict src,
	/*@in@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const restrict user,
	/*@in@*/ const struct LibTTAr_EncMisc *const restrict misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	TTAENC_PARAMS(2u);

	TTAENC_PARAMCHECKS(2u);

	nbytes_enc = tta_encode_2ch_loop(TTAENC_LOOP_ARGS(2u));

	TTAENC_POSTLOOP;
	return retval;
}
#endif	// LIBTTAr_OPT_DISABLE_UNROLLED_2CH

//--------------------------------------------------------------------------//

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
/**@fn tta_encode_2ch_loop
 * @brief unrolled stereo encode loop
 *
 * @see tta_encode_mch_loop()
**/
ALWAYS_INLINE size_t
tta_encode_2ch_loop(
	/*@reldef@*/ u8 *const restrict dest, const i32 *const restrict src,
	u32 *const restrict crc_inout,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache_Enc *const restrict bitcache,
	struct Codec *const restrict codec, const bitcnt_enc predict_k,
	const i32 filter_round, const bitcnt_enc filter_k,
	UNUSED const uint nchan,
	const size_t ni32_target, const size_t write_soft_limit
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
		if ( nbytes_enc > write_soft_limit ){
			break;
		}
	// 0
		curr.i = (next = src[i + 1u]) - src[i + 0u];
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
#endif	// LIBTTAr_OPT_DISABLE_UNROLLED_2CH

// EOF ///////////////////////////////////////////////////////////////////////
