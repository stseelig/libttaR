#ifndef TTA_CODEC_STATE_H
#define TTA_CODEC_STATE_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/state.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stddef.h>

#include "../bits.h"

#include "rice.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

struct LibTTAr_CodecState_Priv {
	struct BitCache	bitcache;
	struct Codec 	codec[];
};

struct LibTTAr_CodecState_User {
	u32	ncalls_codec;
	u32	crc;
	size_t	ni32;			// enc: num read, dec: num written
	size_t	ni32_total;		// ~
	size_t	nbytes_tta;		// enc: num written, dec: num read
	size_t	nbytes_tta_total;	// ~
};

//////////////////////////////////////////////////////////////////////////////

/**@fn state_priv_init
 * @brief initializes a private state struct
 *
 * @param priv[out] the private state struct
 * @param nchan number of audio channels
**/
INLINE void
state_priv_init(
	/*@out@*/
	register struct LibTTAr_CodecState_Priv *const restrict priv,
	register uint nchan
)
/*@modifies	*priv@*/
{
	MEMSET(&priv->bitcache, 0x00, sizeof priv->bitcache);
	codec_init((struct Codec *) &priv->codec, nchan);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
