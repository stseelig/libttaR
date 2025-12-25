/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_enc.c                                                          //
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
#include "./tta_enc.h"
#include "./tta_state.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

#ifndef LIBTTAr_OPT_DISABLE_MCH
#undef dest
#undef priv
#undef user
FLATTEN
static NOINLINE enum LibTTAr_EncRetVal tta_encode_mch(
	/*@reldef@*/ uint8_t *RESTRICT dest,
	/*@in@*/ const int32_t *RESTRICT,
	/*@in@*/ struct LibTTAr_CodecState_Priv *RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *RESTRICT user,
	/*@in@*/ const struct LibTTAr_EncMisc *RESTRICT
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
static NOINLINE enum LibTTAr_EncRetVal tta_encode_1ch(
	/*@reldef@*/ uint8_t *RESTRICT dest,
	/*@in@*/ const int32_t *RESTRICT,
	/*@in@*/ struct LibTTAr_CodecState_Priv *RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *RESTRICT user,
	/*@in@*/ const struct LibTTAr_EncMisc *RESTRICT
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
static NOINLINE enum LibTTAr_EncRetVal tta_encode_2ch(
	/*@reldef@*/ uint8_t *RESTRICT dest,
	/*@in@*/ const int32_t *RESTRICT,
	/*@in@*/ struct LibTTAr_CodecState_Priv *RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *RESTRICT user,
	/*@in@*/ const struct LibTTAr_EncMisc *RESTRICT
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
ALWAYS_INLINE size_t tta_encode_mch_loop(
	/*@reldef@*/ uint8_t *RESTRICT dest, const int32_t *RESTRICT,
	uint32_t *RESTRICT crc_inout, /*@out@*/ size_t *RESTRICT ni32_out,
	struct BitCache_Enc *RESTRICT bitcache, struct Codec *RESTRICT codec,
	bitcnt_enc, int32_t, bitcnt_enc, unsigned int, size_t, size_t
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
ALWAYS_INLINE size_t tta_encode_1ch_loop(
	/*@reldef@*/ uint8_t *RESTRICT dest, const int32_t *RESTRICT,
	uint32_t *RESTRICT crc_inout, /*@out@*/ size_t *RESTRICT ni32_out,
	struct BitCache_Enc *RESTRICT bitcache, struct Codec *RESTRICT codec,
	bitcnt_enc, int32_t, bitcnt_enc, unsigned int, size_t, size_t
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
ALWAYS_INLINE size_t tta_encode_2ch_loop(
	/*@reldef@*/ uint8_t *RESTRICT dest, const int32_t *RESTRICT,
	uint32_t *RESTRICT crc_inout, /*@out@*/ size_t *RESTRICT ni32_out,
	struct BitCache_Enc *RESTRICT bitcache, struct Codec *RESTRICT codec,
	bitcnt_enc, int32_t, bitcnt_enc, unsigned int, size_t, size_t
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

/**@fn libttaR_tta_encode
 * @brief a reentrant TTA encoder
 *
 * @param dest - destination buffer
 * @param src  - source buffer
 * @param priv - private state struct
 * @param user - user readable state struct
 * @param misc - other values/properties
 *
 * @return the state of the encoder
 * @retval LIBTTAr_ERV_OK_DONE       - frame finished
 * @retval LIBTTAr_ERV_OK_AGAIN      - frame did not finish
 * @retval LIBTTAr_ERV_FAIL_OVERFLOW - integer overflow occured
 * @retval LIBTTAr_ERV_INVAL_*       - bad parameter
 * @retval LIBTTAr_ERV_MISCONFIG     - library was misconfigured
 *
 * @pre 'user' initialized with LIBTTAr_CODECSTATE_USER_INIT before 1st call
 *
 * @note read the manpage for more info
**/
BUILD_EXPORT
enum LibTTAr_EncRetVal
libttaR_tta_encode(
	/*@reldef@*/ uint8_t *RESTRICT const dest,
	/*@in@*/ const int32_t *RESTRICT const src,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const RESTRICT user,
	/*@in@*/ const struct LibTTAr_EncMisc *const RESTRICT misc
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
		return LIBTTAr_ERV_INVAL_RANGE;
	}
	if UNLIKELY (
		((uintptr_t) priv) % LIBTTAr_CODECSTATE_PRIV_ALIGN != 0
	){
		return LIBTTAr_ERV_INVAL_ALIGN;
	}

	/* init private state */
	if ( user->ncalls_codec == 0 ){
		state_priv_init_enc(priv, misc->nchan);
	}

	switch ( misc->nchan ){
	default:
#ifndef LIBTTAr_OPT_DISABLE_MCH
		return tta_encode_mch(dest, src, priv, user, misc);
#else
		return LIBTTAr_ERV_MISCONFIG;
#endif	/* LIBTTAr_OPT_DISABLE_MCH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
	case 1u:
		return tta_encode_1ch(dest, src, priv, user, misc);
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_1CH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
	case 2u:
		return tta_encode_2ch(dest, src, priv, user, misc);
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
static NOINLINE enum LibTTAr_EncRetVal
tta_encode_mch(
	/*@reldef@*/ uint8_t *const RESTRICT dest,
	/*@in@*/ const int32_t *const RESTRICT src,
	/*@in@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const RESTRICT user,
	/*@in@*/ const struct LibTTAr_EncMisc *const RESTRICT misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	TTAENC_PARAMS(misc->nchan);

	TTAENC_PARAMCHECKS;

	nbytes_enc = tta_encode_mch_loop(TTAENC_LOOP_ARGS);

	TTAENC_POSTLOOP;
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
static NOINLINE enum LibTTAr_EncRetVal
tta_encode_1ch(
	/*@reldef@*/ uint8_t *const RESTRICT dest,
	/*@in@*/ const int32_t *const RESTRICT src,
	/*@in@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const RESTRICT user,
	/*@in@*/ const struct LibTTAr_EncMisc *const RESTRICT misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	TTAENC_PARAMS(1u);

	TTAENC_PARAMCHECKS;

	nbytes_enc = tta_encode_1ch_loop(TTAENC_LOOP_ARGS);

	TTAENC_POSTLOOP;
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
static NOINLINE enum LibTTAr_EncRetVal
tta_encode_2ch(
	/*@reldef@*/ uint8_t *const RESTRICT dest,
	/*@in@*/ const int32_t *const RESTRICT src,
	/*@in@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const RESTRICT user,
	/*@in@*/ const struct LibTTAr_EncMisc *const RESTRICT misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	TTAENC_PARAMS(2u);

	TTAENC_PARAMCHECKS;

	nbytes_enc = tta_encode_2ch_loop(TTAENC_LOOP_ARGS);

	TTAENC_POSTLOOP;
	return retval;
}
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_2CH */

/* ------------------------------------------------------------------------ */

#ifndef LIBTTAr_OPT_DISABLE_MCH
/**@fn tta_encode_mch_loop
 * @brief multichannel/general encode loop
 *
 * @param dest             - destination buffer
 * @param src              - source buffer
 * @param crc_inout        - current CRC
 * @param ni32_out         - 'user'->ni32
 * @param bitcache         - bitcache data
 * @param codec            - the codec struct array
 * @param predict_k        - arg 'k' for tta_predict1
 * @param filter_round     - arg 'round' for tta_filter
 * @param filter_k arg     - 'k' for tta_filter
 * @param nchan            - number of audio channels
 * @param ni32_target      - target number of src i32 to read
 * @param write_soft_limit - soft limit on the safe number of bytes to write
 * @param rice_enc_max     - debug value for theoretical max rice code size
 *
 * @return number of bytes written to 'dest'
**/
ALWAYS_INLINE size_t
tta_encode_mch_loop(
	/*@reldef@*/ uint8_t *const RESTRICT dest,
	const int32_t *const RESTRICT src, uint32_t *const RESTRICT crc_inout,
	/*@out@*/ size_t *const RESTRICT ni32_out,
	struct BitCache_Enc *const RESTRICT bitcache,
	struct Codec *const RESTRICT codec, const bitcnt_enc predict_k,
	const int32_t filter_round, const bitcnt_enc filter_k,
	const unsigned int nchan,
	const size_t ni32_target, const size_t write_soft_limit
#ifndef NDEBUG
	, const size_t rice_enc_max
#endif	/* NDEBUG */
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
	union { int32_t i; uint32_t u; } curr;
	int32_t prev = 0;
	size_t i;
	unsigned int j;

	for ( i = 0; i < ni32_target; i += nchan ){
		if ( nbytes_enc > write_soft_limit ){
			break;
		}

#ifdef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
		prev = 0;
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_1CH */

		for ( j = 0; j < nchan - 1u; ++j ){
			curr.i = src[i + j + 1u] - src[i + j + 0u];
			TTAENC_ENCODE(j);
		}
		curr.i = src[i + (nchan - 1u)] - (prev / 2);
		TTAENC_ENCODE(j);
	}
	*crc_inout = (uint32_t) crc;
	*ni32_out  = i;
	return nbytes_enc;
}
#endif	/* LIBTTAr_OPT_DISABLE_MCH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
/**@fn tta_encode_1ch_loop
 * @brief unrolled mono encode loop
 *
 * @see tta_encode_mch_loop()
**/
ALWAYS_INLINE size_t
tta_encode_1ch_loop(
	/*@reldef@*/ uint8_t *const RESTRICT dest,
	const int32_t *const RESTRICT src, uint32_t *const RESTRICT crc_inout,
	/*@out@*/ size_t *const RESTRICT ni32_out,
	struct BitCache_Enc *const RESTRICT bitcache,
	struct Codec *const RESTRICT codec, const bitcnt_enc predict_k,
	const int32_t filter_round, const bitcnt_enc filter_k,
	UNUSED const unsigned int nchan,
	const size_t ni32_target, const size_t write_soft_limit
#ifndef NDEBUG
	, const size_t rice_enc_max
#endif	/* NDEBUG */
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
	union { int32_t i; uint32_t u; } curr;
	int32_t prev;
	size_t i;

	for ( i = 0; i < ni32_target; ++i ){
		if ( nbytes_enc > write_soft_limit ){
			break;
		}
		curr.i = src[i];
		TTAENC_ENCODE(0);
	}
	*crc_inout = (uint32_t) crc;
	*ni32_out  = i;
	return nbytes_enc;
}
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_1CH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
/**@fn tta_encode_2ch_loop
 * @brief unrolled stereo encode loop
 *
 * @see tta_encode_mch_loop()
**/
ALWAYS_INLINE size_t
tta_encode_2ch_loop(
	/*@reldef@*/ uint8_t *const RESTRICT dest,
	const int32_t *const RESTRICT src, uint32_t *const RESTRICT crc_inout,
	/*@out@*/ size_t *const RESTRICT ni32_out,
	struct BitCache_Enc *const RESTRICT bitcache,
	struct Codec *const RESTRICT codec, const bitcnt_enc predict_k,
	const int32_t filter_round, const bitcnt_enc filter_k,
	UNUSED const unsigned int nchan,
	const size_t ni32_target, const size_t write_soft_limit
#ifndef NDEBUG
	, const size_t rice_enc_max
#endif	/* NDEBUG */
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
	union { int32_t i; uint32_t u; } curr;
	int32_t prev, next;
	size_t i;

	for ( i = 0; i < ni32_target; i += (size_t) 2u ){
		if ( nbytes_enc > write_soft_limit ){
			break;
		}
	/* 0 */
		curr.i = (next = src[i + 1u]) - src[i + 0u];
		TTAENC_ENCODE(0u);
	/* 1 */
		curr.i = next - (prev / 2);
		TTAENC_ENCODE(1u);
	}
	*crc_inout = (uint32_t) crc;
	*ni32_out  = i;
	return nbytes_enc;
}
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_2CH */

/* EOF //////////////////////////////////////////////////////////////////// */
