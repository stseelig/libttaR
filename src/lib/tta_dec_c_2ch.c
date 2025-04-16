//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_dec_c_2ch.c                                                    //
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
#include "tta_dec.h"

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
#undef dest
#undef crc_inout
#undef ni32_out
#undef bitcache
#undef codec
ALWAYS_INLINE size_t tta_decode_2ch_loop(
	/*@reldef@*/ i32 *dest, const u8 *,
	u32 *restrict crc_inout, /*@out@*/ size_t *restrict ni32_out,
	struct BitCache_Dec *restrict bitcache, struct Codec *restrict codec,
	bitcnt_dec, i32, bitcnt_dec, rice24_dec, uint, size_t, size_t
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
HIDDEN enum LibTTAr_DecRetVal
tta_decode_2ch(
	/*@reldef@*/ i32 *const restrict dest,
	/*@in@*/ const u8 *const restrict src,
	/*@in@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const restrict user,
	/*@in@*/ const struct LibTTAr_DecMisc *const restrict misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	TTADEC_PARAMS(2u);

	TTADEC_PARAMCHECKS;

	nbytes_dec = tta_decode_2ch_loop(TTADEC_LOOP_ARGS);

	TTADEC_POSTLOOP;
	return retval;
}
#endif	// LIBTTAr_OPT_DISABLE_UNROLLED_2CH


//--------------------------------------------------------------------------//

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
/**@fn tta_decode_2ch_loop
 * @brief unrolled stereo decode loop
 *
 * @see tta_decode_mch_loop()
**/
ALWAYS_INLINE size_t
tta_decode_2ch_loop(
	/*@out@*/ i32 *const dest, const u8 *const src,
	u32 *const restrict crc_inout,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache_Dec *const restrict bitcache,
	struct Codec *const restrict codec, const bitcnt_dec predict_k,
	const i32 filter_round, const bitcnt_dec filter_k,
	const rice24_dec unary_lax_limit, UNUSED const uint nchan,
	const size_t ni32_target, const size_t read_soft_limit
#ifndef NDEBUG
	, const size_t rice_dec_max
#endif
)
/*@modifies	*dest,
		*crc_inout,
		*ni32_out,
		*bitcache,
		*codec
@*/
{
	size_t nbytes_dec = 0;
	crc32_dec crc = (crc32_dec) *crc_inout;
	union { i32 i; u32 u; } curr;
	i32 prev;
	size_t i;

	for ( i = 0; i < ni32_target; i += (size_t) 2u ){
		if ( nbytes_dec > read_soft_limit ){
			break;
		}
	// 0
		TTADEC_DECODE(0u);
		TTADEC_FILTER(0u);
		TTADEC_PREDICT(0u);
		prev = curr.i;
	// 1
		TTADEC_DECODE(1u);
		TTADEC_FILTER(1u);
		TTADEC_PREDICT(1u);

		dest[i + 1u] = (curr.i += prev / 2);
		dest[i + 0u] = curr.i - prev;
	}
	*crc_inout = (u32) crc;
	*ni32_out  = i;
	return nbytes_dec;
}
#endif	// LIBTTAr_OPT_DISABLE_UNROLLED_2CH

// EOF ///////////////////////////////////////////////////////////////////////
