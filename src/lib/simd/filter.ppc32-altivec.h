#ifndef TTA_CODEC_SIMD_FILTER_PPC32_ALTIVEC_H
#define TTA_CODEC_SIMD_FILTER_PPC32_ALTIVEC_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/simd/filter.ppc32-altivec.h                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef S_SPLINT_S
#ifndef __ALTIVEC__
#error "no AltiVec"
#endif
#endif // S_SPLINT_S

//////////////////////////////////////////////////////////////////////////////

#include "../../bits.h"

//==========================================================================//

#include "filter.ppc.h"

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
	FILTER_SUM_UPDATE_A;
	FILTER_UPDATE_MB(value);
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
	FILTER_SUM_UPDATE_A;
	retval = value + asr32(round, k);
	FILTER_UPDATE_MB(retval);
	FILTER_WRITE;
	*error = value;

	return retval;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
