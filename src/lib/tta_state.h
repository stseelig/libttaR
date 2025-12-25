#ifndef H_TTA_CODEC_TTA_STATE_H
#define H_TTA_CODEC_TTA_STATE_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tta_state.h                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>

#include "./common.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef priv
ALWAYS_INLINE void state_priv_init_enc(
	/*@out@*/ struct LibTTAr_CodecState_Priv *RESTRICT priv, unsigned int
)
/*@modifies	*priv@*/
;

#undef priv
ALWAYS_INLINE void state_priv_init_dec(
	/*@out@*/ struct LibTTAr_CodecState_Priv *RESTRICT priv, unsigned int
)
/*@modifies	*priv@*/
;

#undef codec
ALWAYS_INLINE void codec_init_enc(
	/*@out@*/ struct Codec *RESTRICT codec, unsigned int
)
/*@modifies	*codec@*/
;

#undef codec
ALWAYS_INLINE void codec_init_dec(
	/*@out@*/ struct Codec *RESTRICT codec, unsigned int
)
/*@modifies	*codec@*/
;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn state_priv_init_enc
 * @brief initializes a private state struct; encode version
 *
 * @param priv  - private state struct
 * @param nchan - number of audio channels
**/
ALWAYS_INLINE void
state_priv_init_enc(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	const unsigned int nchan
)
/*@modifies	*priv@*/
{
	MEMSET(&priv->bitcache, 0x00, sizeof priv->bitcache);
	codec_init_enc((struct Codec *) &priv->codec, nchan);

	return;
}

/**@fn state_priv_init_dec
 * @brief initializes a private state struct; decode version
 *
 * @see state_priv_init_enc()
**/
ALWAYS_INLINE void
state_priv_init_dec(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	const unsigned int nchan
)
/*@modifies	*priv@*/
{
	MEMSET(&priv->bitcache, 0x00, sizeof priv->bitcache);
	codec_init_dec((struct Codec *) &priv->codec, nchan);

	return;
}

/* ------------------------------------------------------------------------ */

/* binexp32p4((bitcnt) 10u) */
#define RICE_INIT_ENC	((struct Rice_Enc) { \
	{ UINT32_C(0x00004000), UINT32_C(0x00004000) }, \
	{ (bitcnt_enc) 10u, (bitcnt_enc) 10u } \
})
#define RICE_INIT_DEC	((struct Rice_Dec) { \
	{ UINT32_C(0x00004000), UINT32_C(0x00004000) }, \
	{ (bitcnt_dec) 10u, (bitcnt_dec) 10u } \
})

/**@fn codec_init_enc
 * @brief initializes an array of 'struct Codec'; encode version
 *
 * @param codec - struct array to initialize
 * @param nchan - number of audio channels
**/
ALWAYS_INLINE void
codec_init_enc(
	/*@out@*/ struct Codec *const RESTRICT codec, const unsigned int nchan
)
/*@modifies	*codec@*/
{
	struct Codec *const RESTRICT codec_a = (
		ASSUME_ALIGNED(codec, LIBTTAr_CODECSTATE_PRIV_ALIGN)
	);
	/* * */
	unsigned int i;

	for ( i = 0; i < nchan; ++i ){
		MEMSET(&codec_a[i].filter, 0x00, sizeof codec_a[i].filter);
		codec_a[i].rice.enc = RICE_INIT_ENC;
		codec_a[i].prev     = 0;
	}
	return;
}

/**@fn codec_init_dec
 * @brief initializes an array of 'struct Codec'; decode version
 *
 * @see codec_init_enc()
**/
ALWAYS_INLINE void
codec_init_dec(
	/*@out@*/ struct Codec *const RESTRICT codec, const unsigned int nchan
)
/*@modifies	*codec@*/
{
	struct Codec *const RESTRICT codec_a = (
		ASSUME_ALIGNED(codec, LIBTTAr_CODECSTATE_PRIV_ALIGN)
	);
	/* * */
	unsigned int i;

	for ( i = 0; i < nchan; ++i ){
		MEMSET(&codec_a[i].filter, 0x00, sizeof codec_a[i].filter);
		codec_a[i].rice.dec = RICE_INIT_DEC;
		codec_a[i].prev     = 0;
	}
	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_TTA_STATE_H */
