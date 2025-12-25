#ifndef H_TTA_CODEC_FILTER_ASSERTS_H
#define H_TTA_CODEC_FILTER_ASSERTS_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter/asserts.h                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <stdint.h>

#include "../types.h"

/* ======================================================================== */

#ifndef S_SPLINT_S

#ifndef __BYTE_ORDER__
#error "'__BYTE_ORDER__' not defined"
#endif	/* __BYTE_ORDER__ */

#ifndef __ORDER_BIG_ENDIAN__
#error "'__ORDER_BIG_ENDIAN__' not defined"
#endif	/* __ORDER_BIG_ENDIAN__ */

#ifndef __ORDER_LITTLE_ENDIAN__
#error "'__ORDER_LITTLE_ENDIAN__' not defined"
#endif	/* __ORDER_LITTLE_ENDIAN__ */

#if (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__) \
 && (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
#error "__BYTE_ORDER__"
#endif	/* __BYTE_ORDER__ */

#endif	/* S_SPLINT_S */

/* //////////////////////////////////////////////////////////////////////// */

#define FILTER_ASSERTS_PRE { \
	assert(((k > 0) && (k <= (bitcnt) 32u)) \
	       && \
	        (round == (int32_t) (0x1u << (k - 1u))) \
	); \
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_FILTER_ASSERTS_H */
