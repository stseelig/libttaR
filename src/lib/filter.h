#ifndef TTA_CODEC_FILTER_H
#define TTA_CODEC_FILTER_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../bits.h"

#include "common.h"

//////////////////////////////////////////////////////////////////////////////

#ifndef LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS

// arm
#if defined(__aarch__) || defined(__aarch64__)
#if defined(__ARM_NEON)
#include "filter/filter.arm.h"
#else
#include "filter/filter._C.h"
#endif

// ppc
#elif defined(__powerpc__) || defined(__powerpc64__)
#if defined(__ALTIVEC__)
#include "filter/filter.ppc.h"
#else
#include "filter/filter._C.h"
#endif

// x86
#elif defined(__i386__) || defined(__x86_64__)
#if defined(__SSE2__)
#include "filter/filter.x86.h"
#else
#include "filter/filter._C.h"
#endif

// C
#else
#include "filter/filter._C.h"
#endif

#else // defined(LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS)
#include "filter/filter._C.h"

#endif // LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS

//////////////////////////////////////////////////////////////////////////////

#ifndef S_SPLINT_S

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
	struct Filter *const restrict filter, i32 value, i32 round, bitcnt k
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
 * @see tta_filter_enc()
**/
ALWAYS_INLINE i32
tta_filter_dec(
	struct Filter *const restrict filter, i32 value, i32 round, bitcnt k
)
/*@modifies	*filter@*/
;

#endif	// S_SPLINT_S

// EOF ///////////////////////////////////////////////////////////////////////
#endif
