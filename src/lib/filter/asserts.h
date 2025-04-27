#ifndef TTA_CODEC_FILTER_ASSERTS_H
#define TTA_CODEC_FILTER_ASSERTS_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter/asserts.h                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2025, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#define FILTER_ASSERTS_PRE { \
	assert(((k > 0) && (k < (bitcnt) 32u)) \
	       && \
	        (round == (i32) (0x1u << (k - 1u))) \
	); \
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
