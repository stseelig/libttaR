#ifndef H_TTA_COMMON_H
#define H_TTA_COMMON_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// common.h                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <limits.h>
#include <stdint.h>

/* ======================================================================== */

#include "./splint.h"

/* ======================================================================== */

#if CHAR_BIT != 8u
#error "CHAR_BIT"
#endif	/* CHAR_BITS */

/* //////////////////////////////////////////////////////////////////////// */

#if SIZE_MAX == UINT32_MAX
#define SIZE_C(x_x)	UINT32_C(x_x)
#elif SIZE_MAX == UINT64_MAX
#define SIZE_C(x_x)	UINT64_C(x_x)
#else
#error "SIZE_MAX"
#endif	/* SIZE_C */

/* //////////////////////////////////////////////////////////////////////// */

#ifdef __GNUC__

#ifdef __has_attribute
#define X_HAS_ATTRIBUTE_GNUC(x_x)	__has_attribute(x_x)
#else
#define X_HAS_ATTRIBUTE_GNUC(x_x)	0
#endif	/* __has_attribute */

#define X_ATTRIBUTE_GNUC_ALWAYS_INLINE	always_inline
#define X_ATTRIBUTE_GNUC_NOINLINE	noinline
#define X_ATTRIBUTE_GNUC_CONST		const
#define X_ATTRIBUTE_GNUC_PURE		pure
#define X_ATTRIBUTE_GNUC_UNUSED		unused
#define X_ATTRIBUTE_GNUC_NORETURN	noreturn
#define X_ATTRIBUTE_GNUC_HOT		hot
#define X_ATTRIBUTE_GNUC_COLD		cold

#define X_ATTRIBUTE_GNUC_PACKED		packed

#else	/* ! defined(__GNUC__) */

#define X_HAS_ATTRIBUTE_GNUC(xx_)	0

#define X_ATTRIBUTE_GNUC_ALWAYS_INLINE	nil
#define X_ATTRIBUTE_GNUC_NOINLINE	nil
#define X_ATTRIBUTE_GNUC_CONST		nil
#define X_ATTRIBUTE_GNUC_PURE		nil
#define X_ATTRIBUTE_GNUC_UNUSED		nil
#define X_ATTRIBUTE_GNUC_NORETURN	nil
#define X_ATTRIBUTE_GNUC_HOT		nil
#define X_ATTRIBUTE_GNUC_COLD		nil

#define X_ATTRIBUTE_GNUC_PACKED		nil

#endif	/* __GNUC__ */

/* ======================================================================== */

#if __STDC_VERSION__ >= 199901L
#define INLINE		/*@unused@*/ static inline
#elif defined(__GNUC__)
#define INLINE		/*@unused@*/ static __inline__
#else
#define INLINE		/*@unused@*/ static
#endif	/* INLINE */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_ALWAYS_INLINE)
#define ALWAYS_INLINE	INLINE __attribute__((X_ATTRIBUTE_GNUC_ALWAYS_INLINE))
#else
#define ALWAYS_INLINE	INLINE
#endif	/* ALWAYS_INLINE */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_NOINLINE)
#define NOINLINE	__attribute__((X_ATTRIBUTE_GNUC_NOINLINE))
#else
#define NOINLINE
#endif	/* NOINLINE */

/* ------------------------------------------------------------------------ */

#if __STDC_VERSION__ >= 199901L
#define RESTRICT	restrict
#elif defined(__GNUC__)
#define RESTRICT	__restrict__
#else
#define RESTRICT
#endif	/* RESTRICT */

/* ------------------------------------------------------------------------ */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_CONST)
#define CONST		__attribute__((X_ATTRIBUTE_GNUC_CONST))
#else
#define CONST
#endif	/* CONST */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_PURE)
#define PURE		__attribute__((X_ATTRIBUTE_GNUC_PURE))
#else
#define PURE
#endif	/* PURE */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_UNUSED)
#define UNUSED	/*@unused@*/ __attribute__((X_ATTRIBUTE_GNUC_UNUSED))
#else
#define UNUSED	/*@unused@*/
#endif	/* UNUSED */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_NORETURN)
#define NORETURN	\
	/*@noreturn@*/ __attribute__((X_ATTRIBUTE_GNUC_NORETURN))
#else
#define NORETURN	/*@noreturn@*/
#endif	/* NORETURN */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_HOT)
#define HOT		__attribute__((X_ATTRIBUTE_GNUC_HOT))
#else
#define HOT
#endif	/* HOT */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_COLD)
#define COLD		__attribute__((X_ATTRIBUTE_GNUC_COLD))
#else
#define COLD
#endif	/* COLD */

/* ------------------------------------------------------------------------ */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_PACKED)
#define PACKED		__attribute__((X_ATTRIBUTE_GNUC_PACKED))
#else
#ifndef S_SPLINT_S
#error "compiler does not support the attribute 'packed'"
#endif	/* S_SPLINT_S */
#define PACKED
#endif	/* PACKED */

/* //////////////////////////////////////////////////////////////////////// */

#ifdef __GNUC__

#ifdef __has_builtin
#define X_HAS_BUILTIN_GNUC(x_x)		__has_builtin(x_x)
#else
#define X_HAS_BUILTIN_GNUC(x_x)		0
#endif	/* __has_builtin */

#define X_BUILTIN_GNUC_EXPECT		__builtin_expect
#define X_BUILTIN_GNUC_EXPECT_WITH_PROB	__builtin_expect_with_probability
#define X_BUILTIN_GNUC_UNPREDICABLE	__builtin_unpredictable

#define X_BUILTIN_GNUC_UNREACHABLE	__builtin_unreachable

#define X_BUILTIN_GNUC_ASSUME_ALIGNED	__builtin_assume_aligned

#else	/* ! defined(__GNUC__) */

#define X_HAS_BUILTIN_GNUC(x_x)		0

#define X_BUILTIN_GNUC_EXPECT		nil
#define X_BUILTIN_GNUC_EXPECT_WITH_PROB	nil
#define X_BUILTIN_GNUC_UNPREDICABLE	nil

#define X_BUILTIN_GNUC_UNREACHABLE	nil

#define X_BUILTIN_GNUC_ASSUME_ALIGNED	nil

#endif	/* __GNUC__ */

/* ======================================================================== */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_EXPECT)
#define LIKELY(x_cond)		(X_BUILTIN_GNUC_EXPECT((x_cond), 0==0))
#define UNLIKELY(x_cond)	(X_BUILTIN_GNUC_EXPECT((x_cond), 0!=0))
#else
#define LIKELY(x_cond)		(x_cond)
#define UNLIKELY(x_cond)	(x_cond)
#endif	/* (UN)LIKELY */

/* ------------------------------------------------------------------------ */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_UNREACHABLE)
#define UNREACHABLE	/*@notreached@*/ X_BUILTIN_GNUC_UNREACHABLE()
#else
#define UNREACHABLE	/*@notreached@*/
#endif	/* UNREACHABLE */

/* ------------------------------------------------------------------------ */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_ASSUME_ALIGNED)
#define ASSUME_ALIGNED(x_ptr, x_align)	\
	X_BUILTIN_GNUC_ASSUME_ALIGNED((x_ptr), (x_align))
#else
#define ASSUME_ALIGNED(x_ptr, x_align)	(x_ptr)
#endif	/* ASSUME_ALIGNED */

/* //////////////////////////////////////////////////////////////////////// */

#ifdef C_BUILD_C
#define BUILD			static
#define BUILD_EXTERN		BUILD /*@external@*/ /*@unused@*/
#else
#define BUILD			/*@unused@*/
#define BUILD_EXTERN		extern /*@external@*/ /*@unused@*/
#endif	/* C_BUILD_C */

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_COMMON_H */
