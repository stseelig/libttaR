#ifndef H_TTA_CODEC_FILTER_STRUCT_H
#define H_TTA_CODEC_FILTER_STRUCT_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter/struct.h                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>

#include "../common.h"

#include "./struct.h"

/* //////////////////////////////////////////////////////////////////////// */

#if 0	/* arch-type */

/* arm */
#elif !defined(LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS) \
 && (defined(__aarch__) || defined(__aarch64__)) && defined(__ARM_NEON)
#define ARM_SIMD_INTRINSICS
#define USING_SIMD_INTRINSICS

/* ppc */
#elif !defined(LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS) \
 && (defined(__powerpc__) || defined(__powerpc64__)) && defined(__ALTIVEC__)
#define PPC_SIMD_INTRINSICS
#define USING_SIMD_INTRINSICS

/* x86 */
#elif !defined(LIBTTAr_OPT_DISABLE_SIMD_INTRINSICS) \
 && (defined(__i386__) || defined(__x86_64__)) && defined(__SSE2__)
#define X86_SIMD_INTRINSICS
#define USING_SIMD_INTRINSICS

#endif	/* arch-type */

/* //////////////////////////////////////////////////////////////////////// */

#ifdef USING_SIMD_INTRINSICS

struct ALIGNED(LIBTTAr_CODECSTATE_PRIV_ALIGN) Filter {
	int32_t	qm[8u];
	int32_t	dx[8u];
	int32_t	dl[8u];
	int32_t	error;	/* the full error */
};

#else	/* !defined(USING_SIMD_INTRINSICS) */

struct ALIGNED(LIBTTAr_CODECSTATE_PRIV_ALIGN) Filter {
	int32_t	qm[8u];
	int32_t	dx[9u];	/* the extra value is for a memmove(3) trick */
	int32_t	dl[9u];	/* ~                                      */
	int32_t	error;	/* sign of the error (-1, 1, or 0)        */
};

#endif	/* USING_SIMD_INTRINSICS */

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_FILTER_STRUCT_H */
