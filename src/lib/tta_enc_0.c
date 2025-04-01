//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_enc_0.c                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../bits.h"

#include "common.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_MCH
#undef dest
#undef priv
#undef user
extern HIDDEN enum LibTTAr_EncRetVal tta_encode_mch(
	/*@reldef@*/ u8 *restrict dest,
	/*@in@*/ const i32 *restrict,
	/*@in@*/ struct LibTTAr_CodecState_Priv *restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *restrict user,
	/*@in@*/ const struct LibTTAr_EncMisc *restrict
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;
#endif	// LIBTTAr_OPT_DISABLE_MCH

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
#undef dest
#undef priv
#undef user
extern HIDDEN enum LibTTAr_EncRetVal tta_encode_1ch(
	/*@reldef@*/ u8 *restrict dest,
	/*@in@*/ const i32 *restrict,
	/*@in@*/ struct LibTTAr_CodecState_Priv *restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *restrict user,
	/*@in@*/ const struct LibTTAr_EncMisc *restrict
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;
#endif	// LIBTTAr_OPT_DISABLE_UNROLLED_1CH

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
#undef dest
#undef priv
#undef user
extern HIDDEN enum LibTTAr_EncRetVal tta_encode_2ch(
	/*@reldef@*/ u8 *restrict dest,
	/*@in@*/ const i32 *restrict,
	/*@in@*/ struct LibTTAr_CodecState_Priv *restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *restrict user,
	/*@in@*/ const struct LibTTAr_EncMisc *restrict
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;
#endif	// LIBTTAr_OPT_DISABLE_UNROLLED_2CH

//////////////////////////////////////////////////////////////////////////////

/**@fn libttaR_tta_encode
 * @brief a reentrant TTA encoder
 *
 * @param dest[out] destination buffer
 * @param src[in] source buffer
 * @param priv[in out] private state struct
 * @param user[in out] user readable state struct
 * @param misc[in] other values/properties
 *
 * @return the state of the encoder
 * @retval LIBTTAr_ERV_DONE the frame finished
 * @retval LIBTTAr_ERV_AGAIN the frame did not finish
 * @retval LIBTTAr_ERV_INVAL_* bad parameter
 * @retval LIBTTAr_ERV_MISCONFIG library was misconfigured
 *
 * @pre 'user' initialized with LIBTTAr_CODECSTATE_USER_INIT before 1st call
 *
 * @note read the manpage for more info
 * @note affected by:
 *     LIBTTAr_OPT_DISABLE_UNROLLED_1CH,
 *     LIBTTAr_OPT_DISABLE_UNROLLED_2CH,
 *     LIBTTAr_OPT_DISABLE_MCH
**/
/*@unused@*/
enum LibTTAr_EncRetVal
libttaR_tta_encode(
	/*@reldef@*/ u8 *restrict const dest,
	/*@in@*/ const i32 *restrict const src,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@in@*/ struct LibTTAr_CodecState_User *const restrict user,
	/*@in@*/ const struct LibTTAr_EncMisc *const restrict misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
{
	// init state
	if ( user->ncalls_codec == 0 ){
		state_priv_init_enc(priv, misc->nchan);
	}

	switch ( misc->nchan ){
	default:
#ifndef LIBTTAr_OPT_DISABLE_MCH
		return tta_encode_mch(dest, src, priv, user, misc);
#else
		return LIBTTAr_ERV_MISCONFIG;
#endif
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
	case 1u:
		return tta_encode_1ch(dest, src, priv, user, misc);
#endif
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
	case 2u:
		return tta_encode_2ch(dest, src, priv, user, misc);
#endif
	}

#if defined(LIBTTAr_OPT_DISABLE_UNROLLED_1CH) \
 && defined(LIBTTAr_OPT_DISABLE_UNROLLED_2CH) \
 && defined(LIBTTAr_OPT_DISABLE_MCH)
#error "misconfigured codec functions, all channel counts disabled"
#endif
}

// EOF ///////////////////////////////////////////////////////////////////////
