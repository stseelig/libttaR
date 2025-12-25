#ifndef H_TTA_CODEC_FILTER_H
#define H_TTA_CODEC_FILTER_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>

#include "./common.h"
#include "./types.h"

#include "./filter/struct.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef filter
#undef value
#undef round
#undef k
/**@fn tta_filter_enc
 * @brief adaptive encoding filter
 *
 * @param filter - filter data for the current channel
 * @param value  - input value to filter
 * @param round  - intial filter sum
 * @param k      - amount to shift the sum by before subtracting from 'value'
 *
 * @return the filtered value
**/
ALWAYS_INLINE int32_t tta_filter_enc(
	struct Filter *RESTRICT filter, int32_t value, int32_t round, bitcnt k
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
ALWAYS_INLINE int32_t tta_filter_dec(
	struct Filter *RESTRICT filter, int32_t value, int32_t round, bitcnt k
)
/*@modifies	*filter@*/
;

/* //////////////////////////////////////////////////////////////////////// */

#if 0	/* arch-type */

/* arm */
#elif !defined(LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS) \
 && defined(ARM_SIMD_INTRINSICS)
#include "filter/filter.arm.h"

/* ppc */
#elif !defined(LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS) \
 && defined(PPC_SIMD_INTRINSICS)
#include "filter/filter.ppc.h"

/* x86 */
#elif !defined(LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS) \
 && defined(X86_SIMD_INTRINSICS)
#include "filter/filter.x86.h"

/* C */
#else
#include "filter/filter._C.h"

#endif	/* arch-type */

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_FILTER_H */
