#ifndef H_TTA_CODEC_COMMON_H
#define H_TTA_CODEC_COMMON_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/common.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <limits.h>
#include <stdint.h>

/* //////////////////////////////////////////////////////////////////////// */

#ifdef LIBTTAr_OPT_SLOW_CPU

#define LIBTTAr_OPT_NO_NATIVE_TZCNT
#define LIBTTAr_OPT_FEWER_FAST_TYPES
#define LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
#define LIBTTAr_OPT_PREFER_LOOKUP_TABLES

#endif /* LIBTTAr_OPT_SLOW_CPU */

/* //////////////////////////////////////////////////////////////////////// */

/* a 16-bit 'size_t' is too small for the library */
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
#define X_ATTRIBUTE_GNUC_FLATTEN	flatten
#define X_ATTRIBUTE_GNUC_CONST		const
#define X_ATTRIBUTE_GNUC_PURE		pure
#define X_ATTRIBUTE_GNUC_VISIBILITY	visibility
#define X_ATTRIBUTE_GNUC_UNUSED		unused

#define X_ATTRIBUTE_GNUC_ALIGNED	aligned

#else	/* ! defined(__GNUC__) */

#define X_HAS_ATTRIBUTE_GNUC(xx_)	0

#define X_ATTRIBUTE_GNUC_ALWAYS_INLINE	nil
#define X_ATTRIBUTE_GNUC_NOINLINE	nil
#define X_ATTRIBUTE_GNUC_FLATTEN	nil
#define X_ATTRIBUTE_GNUC_CONST		nil
#define X_ATTRIBUTE_GNUC_PURE		nil
#define X_ATTRIBUTE_GNUC_VISIBILITY	nil
#define X_ATTRIBUTE_GNUC_UNUSED		nil

#define X_ATTRIBUTE_GNUC_ALIGNED	nil

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

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_FLATTEN)
#define FLATTEN		__attribute__((X_ATTRIBUTE_GNUC_FLATTEN))
#else
#define FLATTEN
#endif	/* FLATTEN */

/* ------------------------------------------------------------------------ */

#if __STDC_VERSION__ >= 201112L
#define THREAD_LOCAL	_Thread_local
#elif defined(__GNUC__)
#define THREAD_LOCAL	__thread
#else
#ifdef S_SPLINT_S
#define THREAD_LOCAL
#else
#error "THREAD_LOCAL; compile with '-std=c11'"
#endif	/* S_SPLINT_S */
#endif	/* THREAD_LOCAL */

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

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_VISIBILITY)
#define X_HIDDEN	__attribute__((X_ATTRIBUTE_GNUC_VISIBILITY("hidden")))
#else
#define X_HIDDEN
#endif	/* X_HIDDEN */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_UNUSED)
#define UNUSED	/*@unused@*/ __attribute__((X_ATTRIBUTE_GNUC_UNUSED))
#else
#define UNUSED	/*@unused@*/
#endif	/* UNUSED */

/* ------------------------------------------------------------------------ */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_ALIGNED)
#define ALIGNED(x_align)	\
	__attribute__((X_ATTRIBUTE_GNUC_ALIGNED(x_align)))
#else
#ifndef S_SPLINT_S
#error "compiler does not support the attribute 'aligned'"
#else
#define ALIGNED(x_align)
#endif	/* S_SPLINT_S */
#endif	/* ALIGNED */

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

/* clang-11 bug:
	the '_inline's cause clang to panic
*/
#define X_BUILTIN_GNUC_MEMMOVE_INLINE	nil
#define X_BUILTIN_GNUC_MEMSET_INLINE	nil
#define X_BUILTIN_GNUC_MEMMOVE		__builtin_memmove
#define X_BUILTIN_GNUC_MEMSET		__builtin_memset

#if UINT_MAX == UINT32_MAX
#define X_BUILTIN_GNUC_CTZ32		__builtin_ctz
#elif ULONG_MAX == UINT32_MAX
#define X_BUILTIN_GNUC_CTZ32		__builtin_ctzl
#else
#define X_BUILTIN_GNUC_CTZ32		nil
#endif	/* X_BUILTIN_GNUC_CTZ32 */

#if UINT_MAX == UINT64_MAX
#define X_BUILTIN_GNUC_CTZ64		__builtin_ctz
#elif ULONG_MAX == UINT64_MAX
#define X_BUILTIN_GNUC_CTZ64		__builtin_ctzl
#elif ULONG_LONG_MAX == UINT64_MAX
#define X_BUILTIN_GNUC_CTZ64		__builtin_ctzll
#else
#define X_BUILTIN_GNUC_CTZ64		nil
#endif	/* X_BUILTIN_GNUC_CTZ64 */

#else	/* ! defined(__GNUC__) */

#define X_HAS_BUILTIN_GNUC(x_x)		0

#define X_BUILTIN_GNUC_EXPECT		nil
#define X_BUILTIN_GNUC_EXPECT_WITH_PROB	nil
#define X_BUILTIN_GNUC_UNPREDICABLE	nil

#define X_BUILTIN_GNUC_UNREACHABLE	nil

#define X_BUILTIN_GNUC_ASSUME_ALIGNED	nil

#define X_BUILTIN_GNUC_MEMMOVE_INLINE	nil
#define X_BUILTIN_GNUC_MEMSET_INLINE	nil
#define X_BUILTIN_GNUC_MEMMOVE		nil
#define X_BUILTIN_GNUC_MEMSET		nil

#define X_BUILTIN_GNUC_CTZ32		nil
#define X_BUILTIN_GNUC_CTZ64		nil

#endif	/* __GNUC__ */

/* ======================================================================== */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_EXPECT)
#define LIKELY(x_cond)		(X_BUILTIN_GNUC_EXPECT((x_cond), 0==0))
#define UNLIKELY(x_cond)	(X_BUILTIN_GNUC_EXPECT((x_cond), 0!=0))
#else
#define LIKELY(x_cond)		(x_cond)
#define UNLIKELY(x_cond)	(x_cond)
#endif	/* (UN)LIKELY */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_EXPECT_WITH_PROB)
#define PROBABLE(x_cond, x_prob)	\
	(X_BUILTIN_GNUC_EXPECT_WITH_PROB((x_cond), 0==0, (x_prob)))
#define IMPROBABLE(x_cond, x_prob)	\
	(X_BUILTIN_GNUC_EXPECT_WITH_PROB((x_cond), 0!=0, 1.0 - (x_prob)))
#else
#define PROBABLE(x_cond, x_prob)	(x_cond)
#define IMPROBABLE(x_cond, x_prob)	(x_cond)
#endif	/* (IM)PROBABLE */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_UNPREDICTABLE)
#define UNPREDICTABLE(x_cond)	(X_BUILTIN_GNUC_UNPREDICTABLE((x_cond))
#else
#define UNPREDICTABLE(x_cond)	(x_cond)
#endif	/* UNPREDICTABLE */

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

/* ------------------------------------------------------------------------ */

/* -nolibc; a decent compiler should do this anyway */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_MEMMOVE_INLINE)
#define MEMMOVE(dest, src, n)	\
	X_BUILTIN_GNUC_MEMMOVE_INLINE((dest), (src), (n))
#elif X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_MEMMOVE)
#define MEMMOVE(dest, src, n)	X_BUILTIN_GNUC_MEMMOVE((dest), (src), (n))
#else
/* gcc-12 bug:
	- will not reach here, even though it does not have a builtin memmove
*/
#pragma message "compiler does not have a builtin 'memmove'"
#include <string.h>
#define MEMMOVE(dest, src, n)	((void) memmove((dest), (src), (n)))
#endif	/* MEMMOVE */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_MEMSET_INLINE)
#define MEMSET(s, c, n)		X_BUILTIN_GNUC_MEMSET_INLINE((s), (c), (n))
#elif X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_MEMSET)
#define MEMSET(s, c, n)		X_BUILTIN_GNUC_MEMSET((s), (c), (n))
#else
#pragma message "compiler does not have a builtin 'memset'"
#include <string.h>
#define MEMSET(s, c, n)		memset((s), (c), (n))
#endif	/* MEMSET */

/* ------------------------------------------------------------------------ */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_CTZ32)
#define BUILTIN_CTZ32(x_x)	X_BUILTIN_GNUC_CTZ32(x_x)
#define HAS_BUILTIN_CTZ32
#endif	/* BUILTIN_CTZ32 */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_CTZ64)
#define BUILTIN_CTZ64(x_x)	X_BUILTIN_GNUC_CTZ64(x_x)
#define HAS_BUILTIN_CTZ64
#endif	/* BUILTIN_CTZ64 */

#if !defined(LIBTTAr_OPT_PREFER_LOOKUP_TABLES) \
 && !defined(LIBTTAr_OPT_NO_NATIVE_TZCNT) \
 && (!defined(HAS_BUILTIN_CTZ32) && !defined(HAS_BUILTIN_CTZ64))
#pragma message "compiler does not have a builtin 'ctz/tzcnt'"
#endif	/* ctz/tzcnt warning */

#if defined(LIBTTAr_OPT_NO_NATIVE_TZCNT) \
 || defined(LIBTTAr_OPT_PREFER_LOOKUP_TABLES) \
 || (!defined(HAS_BUILTIN_CTZ32) && !defined(HAS_BUILTIN_CTZ64))
#define USE_TBCNT8_TABLE
#endif	/* USE_TBCNT8_TABLE */

/* //////////////////////////////////////////////////////////////////////// */

#ifdef C_BUILD_C
#define BUILD_HIDDEN		static
#define BUILD_EXTERN		/*@external@*/ /*@unused@*/
#else
#define BUILD_HIDDEN		X_HIDDEN
#define BUILD_EXTERN		extern /*@external@*/ /*@unused@*/
#endif	/* C_BUILD_C */

#define BUILD_EXPORT		/*@unused@*/

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_COMMON_H */
