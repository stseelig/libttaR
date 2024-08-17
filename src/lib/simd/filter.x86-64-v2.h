#ifndef TTA_CODEC_SIMD_FILTER_X86_64_V2_H
#define TTA_CODEC_SIMD_FILTER_X86_64_V2_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/simd/filter.x86-64-v2.h                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef S_SPLINT_S
#ifndef __SSE2__
#error "no SSE2"
#endif
#ifndef __SSSE3__
#error "no SSSE3"
#endif
#ifndef __SSE4_1__
#error "no SSE4.1"
#endif
#endif // S_SPLINT_S

//////////////////////////////////////////////////////////////////////////////

#include "../../bits.h"

//==========================================================================//

#include "filter.x86.h"

//==========================================================================//

#include "../common.h"
#include "../tta.h"

//////////////////////////////////////////////////////////////////////////////

///@see "../filter.h"
ALWAYS_INLINE i32
tta_filter_enc(
	struct Filter *const restrict filter, const i32 value, i32 round,
	const bitcnt k
)
/*@modifies	*filter@*/
{
	FILTER_VARIABLES;

	FILTER_READ;
	FILTER_SUM_UPDATE_A(v2);
	FILTER_UPDATE_MB(v2, value);
	FILTER_WRITE;
	retval = value - asr32(round, k);
	*error = retval;
	return retval;
}

///@see "../filter.h"
ALWAYS_INLINE i32
tta_filter_dec(
	struct Filter *const restrict filter, const i32 value, i32 round,
	const bitcnt k
)
/*@modifies	*filter@*/
{
	FILTER_VARIABLES;

	FILTER_READ;
	FILTER_SUM_UPDATE_A(v2);
	retval = value + asr32(round, k);
	FILTER_UPDATE_MB(v2, retval);
	FILTER_WRITE;
	*error = value;
	return retval;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
