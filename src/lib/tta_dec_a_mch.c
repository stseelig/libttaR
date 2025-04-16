//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_dec_a_mch.c                                                    //
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

#ifndef LIBTTAr_OPT_DISABLE_MCH
#undef dest
#undef crc_inout
#undef ni32_out
#undef bitcache
#undef codec
ALWAYS_INLINE size_t tta_decode_mch_loop(
	/*@reldef@*/ i32 *const dest, const u8 *,
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
#endif	// LIBTTAr_OPT_DISABLE_MCH

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_MCH
/**@fn tta_encode_mch
 * @brief multi-channel encoder
 *
 * @see libttaR_tta_encode()
**/
HIDDEN enum LibTTAr_DecRetVal
tta_decode_mch(
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
	TTADEC_PARAMS(misc->nchan);

	TTADEC_PARAMCHECKS;

	nbytes_dec = tta_decode_mch_loop(TTADEC_LOOP_ARGS);

	TTADEC_POSTLOOP;
	return retval;
}
#endif	// LIBTTAr_OPT_DISABLE_MCH

#ifndef LIBTTAr_OPT_DISABLE_MCH
/**@fn tta_decode_mch_loop
 * @brief multichannel/general decode loop
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param crc_inout[in out] the current CRC
 * @param ni32_out[out] 'user'->ni32
 * @param bitcache[in out] the bitcache data
 * @param codec[in out] the codec struct array
 * @param predict_k arg 'k' for tta_predict1
 * @param filter_round arg 'round' for tta_filter
 * @param filter_k arg 'k' for tta_filter
 * @param unary_lax_limit limit for the unary code
 * @param nchan number of audio channels
 * @param ni32_target target number of dest i32 to write
 * @param read_soft_limit soft limit on the safe number of TTA bytes to read
 * @param rice_dec_max debug value for theoretical max unary/binary code size
 *
 * @return number of bytes read from 'src'
 *
 * @note affected by LIBTTAr_OPT_DISABLE_UNROLLED_1CH
**/
ALWAYS_INLINE size_t
tta_decode_mch_loop(
	/*@reldef@*/ i32 *const dest, const u8 *const src,
	u32 *const restrict crc_inout,
	/*@out@*/ size_t *const restrict ni32_out,
	struct BitCache_Dec *const restrict bitcache,
	struct Codec *const restrict codec, const bitcnt_dec predict_k,
	const i32 filter_round, const bitcnt_dec filter_k,
	const rice24_dec unary_lax_limit, const uint nchan,
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
	i32 prev = 0;
	size_t i;
	uint j;

	for ( i = 0; i < ni32_target; i += nchan ){
		if ( nbytes_dec > read_soft_limit ){
			break;
		}
		j = 0;
		goto loop1_entr;
		do {	// decorrelate (1st pass, forwards)
			dest[i + j++] = (prev = curr.i);
loop1_entr:
			TTADEC_DECODE(j);
			TTADEC_FILTER(j);
			TTADEC_PREDICT(j);
		}
		while PROBABLE ( j + 1u < nchan, 0.9 );

		// decorrelate (2nd pass, backwards)
		dest[i + j] = (curr.i += prev / 2);
		for ( j = nchan - 1u; j-- != 0; ){
			dest[i + j] = (curr.i -= dest[i + j]);
		}
	}
	*crc_inout = (u32) crc;
	*ni32_out  = i;
	return nbytes_dec;
}
#endif	// LIBTTAr_OPT_DISABLE_MCH

// EOF ///////////////////////////////////////////////////////////////////////
