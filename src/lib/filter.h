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

#include "../bits.h"

#include "common.h"

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS

// ppc
#if defined(__powerpc__)
#if defined(__ALTIVEC__)
#include "simd/filter.ppc.h"
#else
#include "simd/filter._C.h"
#endif

// x86
#elif defined(__i386__) || defined(__x86_64__)
#if defined(__SSE2__)
#include "simd/filter.x86.h"
#else
#include "simd/filter._C.h"
#endif

// C
#else
#include "simd/filter._C.h"
#endif

#else // defined(LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS)
#include "simd/filter._C.h"

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
