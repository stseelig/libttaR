#ifndef TTA_CODEC_STATE_H
#define TTA_CODEC_STATE_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/state.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stdlib.h>

#include "../bits.h"

#include "rice.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

struct LibTTAr_CodecState_Priv {
	struct BitCache	bitcache;
	struct Codec 	codec[];
};

struct LibTTAr_CodecState_User {

	// set by user
	size_t	ni32_perframe;		// framelen * nchan
	bool	is_new_frame;

	// set by called function
	bool	frame_is_finished;
	u32	crc;
	size_t	ni32;			// enc: num read, dec: num written
	size_t	ni32_total;		// ~
	size_t	nbytes_tta;		// enc: num written, dec: num read
	size_t	nbytes_tta_total;	// ~
};

//////////////////////////////////////////////////////////////////////////////

#undef priv
#undef user
INLINE void state_init_check(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@partial@*/ struct LibTTAr_CodecState_User *const restrict user,
	uint
)
/*@modifies	*priv,
		user->is_new_frame,
		user->frame_is_finished,
		user->crc,
		user->ni32,
		user->ni32_total,
		user->nbytes_tta,
		user->nbytes_tta_total
@*/
;

//////////////////////////////////////////////////////////////////////////////

INLINE void
state_init(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@partial@*/ struct LibTTAr_CodecState_User *const restrict user,
	uint nchan
)
/*@modifies	*priv,
		user->is_new_frame,
		user->frame_is_finished,
		user->crc,
		user->ni32,
		user->ni32_total,
		user->nbytes_tta,
		user->nbytes_tta_total
@*/
{
	if ( user->is_new_frame ){
		user->is_new_frame	= false;
		user->frame_is_finished	= false;
		user->crc		= CRC32_INIT;
		user->ni32		= 0;
		user->ni32_total	= 0;
		user->nbytes_tta	= 0;
		user->nbytes_tta_total	= 0;

		(void) memset(&priv->bitcache, 0x00, sizeof priv->bitcache);
		codec_init((struct Codec *) &priv->codec, nchan);
	}
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
