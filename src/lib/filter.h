#ifndef TTA_CODEC_FILTER_H
#define TTA_CODEC_FILTER_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS

#ifdef __x86_64__

#ifdef __AVX2__
#include "filter/filter.x86-64-v3.h"
#else
#include "filter/filter.C.h"
#endif

#else // C

#include "filter/filter.C.h"

#endif
#else // defined(LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS)

#include "filter/filter.C.h"

#endif // LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS

//////////////////////////////////////////////////////////////////////////////

#undef filter
#undef value
#undef round
#undef k
/**@fn tta_filter_enc
 * @brief adaptive encoding filter
 *
 * @param filter[in out] the filter data for the current channel
 * @param value the input value to filter
 * @param round the intial filter sum
 * @param k amount to shift the sum by before subtracting from 'value'
 *
 * @return the filtered value
**/
ALWAYS_INLINE i32
tta_filter_enc(
	struct Filter *const restrict filter, const i32 value, i32 round,
	const bitcnt k
)
/*@modifies	*filter@*/
;

#undef filter
#undef value
#undef round
#undef k
/**@fn tta_filter_dec
 * @brief adaptive decoding filter
 *
 * @param filter[in out] the filter data for the current channel
 * @param value the input value to filter
 * @param round the intial filter sum
 * @param k amount to shift the sum by before adding to 'value'
 *
 * @return the filtered value
**/
ALWAYS_INLINE i32
tta_filter_dec(
	struct Filter *const restrict filter, const i32 value, i32 round,
	const bitcnt k
)
/*@modifies	*filter@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
