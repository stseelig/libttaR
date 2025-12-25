/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_dec.c                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stddef.h>
#include <stdint.h>

#include "./common.h"
#include "./tta.h"
#include "./tta_dec.h"
#include "./tta_state.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

#ifndef LIBTTAr_OPT_DISABLE_MCH
#undef dest
#undef priv
#undef user
FLATTEN
static NOINLINE enum LibTTAr_DecRetVal tta_decode_mch(
	/*@reldef@*/ int32_t *RESTRICT dest,
	/*@in@*/ const uint8_t *RESTRICT,
	/*@in@*/ struct LibTTAr_CodecState_Priv *RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *RESTRICT user,
	/*@in@*/ const struct LibTTAr_DecMisc *RESTRICT
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;
#endif	/* LIBTTAr_OPT_DISABLE_MCH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
#undef dest
#undef priv
#undef user
FLATTEN
static NOINLINE enum LibTTAr_DecRetVal tta_decode_1ch(
	/*@reldef@*/ int32_t *RESTRICT dest,
	/*@in@*/ const uint8_t *RESTRICT,
	/*@in@*/ struct LibTTAr_CodecState_Priv *RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *RESTRICT user,
	/*@in@*/ const struct LibTTAr_DecMisc *RESTRICT
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_1CH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
#undef dest
#undef priv
#undef user
FLATTEN
static NOINLINE enum LibTTAr_DecRetVal tta_decode_2ch(
	/*@reldef@*/ int32_t *RESTRICT dest,
	/*@in@*/ const uint8_t *RESTRICT,
	/*@in@*/ struct LibTTAr_CodecState_Priv *RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *RESTRICT user,
	/*@in@*/ const struct LibTTAr_DecMisc *RESTRICT
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_2CH */

/* ------------------------------------------------------------------------ */

#ifndef LIBTTAr_OPT_DISABLE_MCH
#undef dest
#undef crc_inout
#undef ni32_out
#undef bitcache
#undef codec
ALWAYS_INLINE size_t tta_decode_mch_loop(
	/*@reldef@*/ int32_t *const dest, const uint8_t *,
	uint32_t *RESTRICT crc_inout, /*@out@*/ size_t *RESTRICT ni32_out,
	struct BitCache_Dec *RESTRICT bitcache, struct Codec *RESTRICT codec,
	bitcnt_dec, int32_t, bitcnt_dec, rice24_dec, unsigned int, size_t,
	size_t
#ifndef NDEBUG
	, size_t
#endif	/* NDEBUG */
)
/*@modifies	*dest,
		*crc_inout,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif	/* LIBTTAr_OPT_DISABLE_MCH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
#undef dest
#undef crc_inout
#undef ni32_out
#undef bitcache
#undef codec
ALWAYS_INLINE size_t tta_decode_1ch_loop(
	/*@reldef@*/ int32_t *const dest, const uint8_t *,
	uint32_t *RESTRICT crc_inout, /*@out@*/ size_t *RESTRICT ni32_out,
	struct BitCache_Dec *RESTRICT bitcache, struct Codec *RESTRICT codec,
	bitcnt_dec, int32_t, bitcnt_dec, rice24_dec, unsigned int, size_t,
	size_t
#ifndef NDEBUG
	, size_t
#endif	/* NDEBUG */
)
/*@modifies	*dest,
		*crc_inout,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_1CH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
#undef dest
#undef crc_inout
#undef ni32_out
#undef bitcache
#undef codec
ALWAYS_INLINE size_t tta_decode_2ch_loop(
	/*@reldef@*/ int32_t *dest, const uint8_t *,
	uint32_t *RESTRICT crc_inout, /*@out@*/ size_t *RESTRICT ni32_out,
	struct BitCache_Dec *RESTRICT bitcache, struct Codec *RESTRICT codec,
	bitcnt_dec, int32_t, bitcnt_dec, rice24_dec, unsigned int, size_t,
	size_t
#ifndef NDEBUG
	, size_t
#endif	/* NDEBUG */
)
/*@modifies	*dest,
		*crc_inout,
		*ni32_out,
		*bitcache,
		*codec
@*/
;
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_2CH */

/* //////////////////////////////////////////////////////////////////////// */

/**@fn libttaR_tta_decode
 * @brief a reentrant TTA decoder
 *
 * @param dest - destination buffer
 * @param src  - source buffer
 * @param priv - private state struct
 * @param user - user readable state struct
 * @param misc - other values/properties
 *
 * @return the state of the decoder
 * @retval LIBTTAr_DRV_OK_DONE       - frame finished
 * @retval LIBTTAr_DRV_OK_AGAIN      - frame did not finish
 * @retval LIBTTAr_DRV_FAIL_DECODE   - frame failed to decode
 * @retval LIBTTAr_DRV_FAIL_OVERFLOW - integer overflow occured
 * @retval LIBTTAr_DRV_INVAL_*       - bad parameter
 * @retval LIBTTAr_DRV_MISCONFIG     - library was misconfigured
 *
 * @pre 'user' initialized with LIBTTAr_CODECSTATE_USER_INIT before 1st call
 *
 * @note read the manpage for more info
**/
BUILD_EXPORT
enum LibTTAr_DecRetVal
libttaR_tta_decode(
	/*@reldef@*/ int32_t *RESTRICT const dest,
	/*@in@*/ const uint8_t *RESTRICT const src,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const RESTRICT user,
	/*@in@*/ const struct LibTTAr_DecMisc *const RESTRICT misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	/* UB-prevention checks */
	if UNLIKELY (
	     ((unsigned int) misc->samplebytes == 0)
	    ||
	     ((unsigned int) misc->samplebytes > LIBTTAr_SAMPLEBYTES_MAX)
	){
		return LIBTTAr_DRV_INVAL_RANGE;
	}
	if UNLIKELY (
		((uintptr_t) priv) % LIBTTAr_CODECSTATE_PRIV_ALIGN != 0
	){
		return LIBTTAr_DRV_INVAL_ALIGN;
	}

	/* init private state */
	if ( user->ncalls_codec == 0 ){
		state_priv_init_dec(priv, misc->nchan);
	}

	switch ( misc->nchan ){
	default:
#ifndef LIBTTAr_OPT_DISABLE_MCH
		return tta_decode_mch(dest, src, priv, user, misc);
#else
		return LIBTTAr_DRV_MISCONFIG;
#endif	/* LIBTTAr_OPT_DISABLE_MCH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
	case 1u:
		return tta_decode_1ch(dest, src, priv, user, misc);
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_1CH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
	case 2u:
		return tta_decode_2ch(dest, src, priv, user, misc);
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_2CH */
	}

#if defined(LIBTTAr_OPT_DISABLE_UNROLLED_1CH) \
 && defined(LIBTTAr_OPT_DISABLE_UNROLLED_2CH) \
 && defined(LIBTTAr_OPT_DISABLE_MCH)
#error "misconfigured codec functions, all channel counts disabled"
#endif	/* misconfig check */
}

/* ------------------------------------------------------------------------ */

#ifndef LIBTTAr_OPT_DISABLE_MCH
/**@fn tta_encode_mch
 * @brief multi-channel encoder
 *
 * @see libttaR_tta_encode()
**/
FLATTEN
static NOINLINE enum LibTTAr_DecRetVal
tta_decode_mch(
	/*@reldef@*/ int32_t *const RESTRICT dest,
	/*@in@*/ const uint8_t *const RESTRICT src,
	/*@in@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const RESTRICT user,
	/*@in@*/ const struct LibTTAr_DecMisc *const RESTRICT misc
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
#endif	/* LIBTTAr_OPT_DISABLE_MCH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
/**@fn tta_encode_1ch
 * @brief mono encoder
 *
 * @see libttaR_tta_encode()
**/
FLATTEN
static NOINLINE enum LibTTAr_DecRetVal
tta_decode_1ch(
	/*@reldef@*/ int32_t *const RESTRICT dest,
	/*@in@*/ const uint8_t *const RESTRICT src,
	/*@in@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const RESTRICT user,
	/*@in@*/ const struct LibTTAr_DecMisc *const RESTRICT misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	TTADEC_PARAMS(1u);

	TTADEC_PARAMCHECKS;

	nbytes_dec = tta_decode_1ch_loop(TTADEC_LOOP_ARGS);

	TTADEC_POSTLOOP;
	return retval;
}
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_1CH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
/**@fn tta_encode_2ch
 * @brief stereo encoder
 *
 * @see libttaR_tta_encode()
**/
FLATTEN
static NOINLINE enum LibTTAr_DecRetVal
tta_decode_2ch(
	/*@reldef@*/ int32_t *const RESTRICT dest,
	/*@in@*/ const uint8_t *const RESTRICT src,
	/*@in@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const RESTRICT user,
	/*@in@*/ const struct LibTTAr_DecMisc *const RESTRICT misc
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
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_2CH */

/* ------------------------------------------------------------------------ */

#ifndef LIBTTAr_OPT_DISABLE_MCH
/**@fn tta_decode_mch_loop
 * @brief multichannel/general decode loop
 *
 * @param dest            - destination buffer
 * @param src             - source buffer
 * @param crc_inout       - current CRC
 * @param ni32_out        - 'user'->ni32
 * @param bitcache        - bitcache data
 * @param codec           - codec struct array
 * @param predict_k       - arg 'k' for tta_predict1
 * @param filter_round    - arg 'round' for tta_filter
 * @param filter_k        - arg 'k' for tta_filter
 * @param unary_lax_limit - limit for the unary code
 * @param nchan           - number of audio channels
 * @param ni32_target     - target number of dest i32 to write
 * @param read_soft_limit - soft limit on the safe number of bytes to read
 * @param rice_dec_max    - debug value for theoretical max rice code size
 *
 * @return number of bytes read from 'src'
**/
ALWAYS_INLINE size_t
tta_decode_mch_loop(
	/*@reldef@*/ int32_t *const dest, const uint8_t *const src,
	uint32_t *const RESTRICT crc_inout,
	/*@out@*/ size_t *const RESTRICT ni32_out,
	struct BitCache_Dec *const RESTRICT bitcache,
	struct Codec *const RESTRICT codec, const bitcnt_dec predict_k,
	const int32_t filter_round, const bitcnt_dec filter_k,
	const rice24_dec unary_lax_limit, const unsigned int nchan,
	const size_t ni32_target, const size_t read_soft_limit
#ifndef NDEBUG
	, const size_t rice_dec_max
#endif	/* NDEBUG */
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
	union { int32_t i; uint32_t u; } curr;
	int32_t prev = 0;
	size_t i;
	unsigned int j;

	for ( i = 0; i < ni32_target; i += nchan ){
		if ( nbytes_dec > read_soft_limit ){
			break;
		}
		j = 0;
		goto loop1_entr;
		do {	/* decorrelate (1st pass, forwards) */
			dest[i + j++] = (prev = curr.i);
loop1_entr:
			TTADEC_DECODE(j);
		}
		while PROBABLE ( j + 1u < nchan, 0.9 );

		/* decorrelate (2nd pass, backwards) */
		dest[i + j] = (curr.i += prev / 2);
		for ( j = nchan - 1u; j-- != 0; ){
			dest[i + j] = (curr.i -= dest[i + j]);
		}
	}
	*crc_inout = (uint32_t) crc;
	*ni32_out  = i;
	return nbytes_dec;
}
#endif	/* LIBTTAr_OPT_DISABLE_MCH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
/**@fn tta_decode_1ch_loop
 * @brief unrolled mono decode loop
 *
 * @see tta_decode_mch_loop()
**/
ALWAYS_INLINE size_t
tta_decode_1ch_loop(
	/*@reldef@*/ int32_t *const dest, const uint8_t *const src,
	uint32_t *const RESTRICT crc_inout,
	/*@out@*/ size_t *const RESTRICT ni32_out,
	struct BitCache_Dec *const RESTRICT bitcache,
	struct Codec *const RESTRICT codec, const bitcnt_dec predict_k,
	const int32_t filter_round, const bitcnt_dec filter_k,
	const rice24_dec unary_lax_limit, UNUSED const unsigned int nchan,
	const size_t ni32_target, const size_t read_soft_limit
#ifndef NDEBUG
	, const size_t rice_dec_max
#endif	/* NDEBUG */
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
	union { int32_t i; uint32_t u; } curr;
	size_t i;

	for ( i = 0; i < ni32_target; ++i ){
		if ( nbytes_dec > read_soft_limit ){
			break;
		}
		TTADEC_DECODE(0);
		dest[i] = curr.i;
	}
	*crc_inout = (uint32_t) crc;
	*ni32_out  = i;
	return nbytes_dec;
}
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_1CH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
/**@fn tta_decode_2ch_loop
 * @brief unrolled stereo decode loop
 *
 * @see tta_decode_mch_loop()
**/
ALWAYS_INLINE size_t
tta_decode_2ch_loop(
	/*@reldef@*/ int32_t *const dest, const uint8_t *const src,
	uint32_t *const RESTRICT crc_inout,
	/*@out@*/ size_t *const RESTRICT ni32_out,
	struct BitCache_Dec *const RESTRICT bitcache,
	struct Codec *const RESTRICT codec, const bitcnt_dec predict_k,
	const int32_t filter_round, const bitcnt_dec filter_k,
	const rice24_dec unary_lax_limit, UNUSED const unsigned int nchan,
	const size_t ni32_target, const size_t read_soft_limit
#ifndef NDEBUG
	, const size_t rice_dec_max
#endif	/* NDEBUG */
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
	union { int32_t i; uint32_t u; } curr;
	int32_t prev;
	size_t i;

	for ( i = 0; i < ni32_target; i += (size_t) 2u ){
		if ( nbytes_dec > read_soft_limit ){
			break;
		}
	/* 0 */
		TTADEC_DECODE(0u);
		prev = curr.i;
	/* 1 */
		TTADEC_DECODE(1u);

		dest[i + 1u] = (curr.i += prev / 2);
		dest[i + 0u] = curr.i - prev;
	}
	*crc_inout = (uint32_t) crc;
	*ni32_out  = i;
	return nbytes_dec;
}
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_2CH */

/* EOF //////////////////////////////////////////////////////////////////// */
