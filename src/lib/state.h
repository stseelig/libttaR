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

// EOF ///////////////////////////////////////////////////////////////////////
#endif
