#ifndef TTA_BITS_H
#define TTA_BITS_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// bits.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>	// true, false
#include <stdint.h>

//////////////////////////////////////////////////////////////////////////////

typedef unsigned char	uchar;
typedef unsigned int	uint;

typedef signed char	ichar;
typedef long long	longlong;

typedef  uint8_t	 u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef uint64_t	u64;

typedef   int8_t	 i8;
typedef  int32_t	i32;

typedef uint_fast8_t	 u8f;
typedef uint_fast32_t	u32f;
typedef uint_fast64_t	u64f;

//////////////////////////////////////////////////////////////////////////////

// checks if the targeted arch has an signed (arithmetic) right shift
#define HAS_ASR(Xtype)	( \
	/*@-shiftimplementation@*/ \
	(Xtype) (((Xtype) UINTMAX_MAX) >> 1u) == (Xtype) UINTMAX_MAX \
	/*@=shiftimplementation@*/ \
)

//==========================================================================//

#ifdef __GNUC__

#ifdef __has_attribute
#define HAS_ATTRIBUTE(x)	__has_attribute(x)
#else
#define HAS_ATTRIBUTE(x)	0
#endif

#else // ! defined(__GNUC__)

#define HAS_ATTRIBUTE(x)	0

#endif // __GNUC__

//--------------------------------------------------------------------------//

#define INLINE			/*@unused@*/ static inline

#if HAS_ATTRIBUTE(always_inline)
#define ALWAYS_INLINE		INLINE __attribute__((always_inline))
#else
#pragma message "compiler does not support the attribute 'always_inline'"
#define ALWAYS_INLINE		INLINE
#endif

#if HAS_ATTRIBUTE(noinline)
#define NOINLINE		__attribute__((noinline))
#else
#define NOINLINE
#endif

#if HAS_ATTRIBUTE(hot)
#define HOT			__attribute__((hot))
#else
#define HOT
#endif

#if HAS_ATTRIBUTE(cold)
#define COLD			__attribute__((cold))
#else
#define COLD
#endif

#if HAS_ATTRIBUTE(pure)
#define PURE			__attribute__((pure))
#else
#define PURE
#endif

#if HAS_ATTRIBUTE(const)
#define CONST			__attribute__((const))
#else
#define CONST
#endif

#if HAS_ATTRIBUTE(noreturn)
#define NORETURN		/*@noreturn@*/ __attribute__((noreturn))
#else
#define NORETURN		/*@noreturn@*/
#endif

#if HAS_ATTRIBUTE(unused)
#define UNUSED			/*@unused@*/   __attribute__((unused))
#else
#define UNUSED			/*@unused@*/
#endif

#if HAS_ATTRIBUTE(aligned)
#define ALIGNED(x)			__attribute__((aligned(x)))
#else
#ifndef S_SPLINT_S
#error "compiler does not support the attribute 'aligned'"
#else
#define ALIGNED(x)
#endif	// S_SPLINT_S
#endif

#if HAS_ATTRIBUTE(packed)
#define PACKED			__attribute__((packed))
#else
#ifndef S_SPLINT_S
#error "compiler does not support the attribute 'packed'"
#else
#define PACKED
#endif	// S_SPLINT_S
#endif

#if HAS_ATTRIBUTE(visibility)
#define HIDDEN			__attribute__((visibility("hidden")))
#else
#define HIDDEN
#endif

//==========================================================================//

#ifdef __GNUC__

#ifdef __has_builtin
#define HAS_BUILTIN(x)			__has_builtin(x)
#else
#define HAS_BUILTIN(x)			0
#endif

#define BUILTIN_EXPECT			__builtin_expect
#define BUILTIN_EXPECT_WITH_PROBABILITY	__builtin_expect_with_probability
#define BUILTIN_UNPREDICTABLE		__builtin_unpredictable

#define BUILTIN_UNREACHABLE		__builtin_unreachable

#define BUILTIN_ASSUME_ALIGNED		__builtin_assume_aligned

#define BUILTIN_BSWAP16			__builtin_bswap16
#define BUILTIN_BSWAP32			__builtin_bswap32
#define BUILTIN_BSWAP64			__builtin_bswap64

#else // ! defined(__GNUC__)

#define HAS_BUILTIN(x)			0

#define BUILTIN_EXPECT			nil
#define BUILTIN_EXPECT_WITH_PROBABILITY	nil
#define BUILTIN_UNPREDICTABLE		nil

#define BUILTIN_UNREACHABLE		nil

#define BUILTIN_ASSUME_ALIGNED		nil

#define BUILTIN_BSWAP16			nil
#define BUILTIN_BSWAP32			nil
#define BUILTIN_BSWAP64			nil

#endif // __GNUC__

//////////////////////////////////////////////////////////////////////////////

// these should always be A/B tested; not always beneficial

#if HAS_BUILTIN(BUILTIN_EXPECT)
#define LIKELY(cond)		(BUILTIN_EXPECT((cond), true))
#define UNLIKELY(cond)		(BUILTIN_EXPECT((cond), false))
#else
#pragma message "compiler does not have a builtin 'expect'"
#define LIKELY(cond)		(cond)
#define UNLIKELY(cond)		(cond)
#endif

#if HAS_BUILTIN(BUILTIN_EXPECT_WITH_PROBABILITY)
#define PROBABLE(cond, prob)	( \
	BUILTIN_EXPECT_WITH_PROBABILITY((cond), true, (prob)) \
)
#define IMPROBABLE(cond, prob)	( \
	BUILTIN_EXPECT_WITH_PROBABILITY((cond), false, 1.0 - (prob)) \
)
#else
#pragma message "compiler does not have a builtin 'expect_with_probability'"
#define PROBABLE(cond, prob)	(cond)
#define IMPROBABLE(cond, prob)	(cond)
#endif

// not sure if this is useful
#if HAS_BUILTIN(BUILTIN_UNPREDICTABLE)
#define UNPREDICTABLE(cond)	(BUILTIN_UNPREDICTABLE(cond))
#else
//#pragma message "compiler does not have a builtin 'unpredictable'"
#define UNPREDICTABLE(cond)	(cond)
#endif

//--------------------------------------------------------------------------//

// mainly for byte-shaving errors

#if HAS_BUILTIN(BUILTIN_UNREACHABLE)
#define UNREACHABLE		BUILTIN_UNREACHABLE(); /*@notreached@*/
#else
#define UNREACHABLE		; /*@notreached@*/
#endif

//////////////////////////////////////////////////////////////////////////////

#if HAS_BUILTIN(BUILTIN_ASSUME_ALIGNED)
#define ASSUME_ALIGNED(x, align)	BUILTIN_ASSUME_ALIGNED((x), (align))
#else
#pragma message "compiler does not have a builtin 'assume_aligned'"
#define ASSUME_ALIGNED(x, align)	(x)
#endif

// workaround for C99 not having max_align_t
union max_alignment {
	long long ll;
	long double ld;
	void *p;
	void(*fp)(void);
};
#define MAX_ALIGNMENT	((size_t) sizeof(union max_alignment))

#ifdef __GNUC__
#define ALIGNOF(x)	__alignof__(x)
#else
#pragma message "compiler does not have an 'alignof'"
#define ALIGNOF(x)	MAX_ALIGNMENT
#endif

#define ALIGN(size, alignment)	((size_t) ( \
	((size) % (alignment)) != 0 \
		? (alignment) - ((size) % (alignment)) : 0 \
))

//////////////////////////////////////////////////////////////////////////////

#undef bswap16
#undef bswap32
#undef bswap64

#undef letoh16
#undef htole16
#undef letoh32
#undef htole32
#undef letoh64
#undef htole64

ALWAYS_INLINE CONST u16 bswap16(u16) /*@*/;
ALWAYS_INLINE CONST u32 bswap32(u32) /*@*/;
ALWAYS_INLINE CONST u64 bswap64(u64) /*@*/;

ALWAYS_INLINE CONST u16 htole16(u16) /*@*/;
ALWAYS_INLINE CONST u16 letoh16(u16) /*@*/;
ALWAYS_INLINE CONST u32 htole32(u32) /*@*/;
ALWAYS_INLINE CONST u32 letoh32(u32) /*@*/;
ALWAYS_INLINE CONST u64 htole64(u64) /*@*/;
ALWAYS_INLINE CONST u64 letoh64(u64) /*@*/;

//////////////////////////////////////////////////////////////////////////////

/**@fn bswap16
 * @brief byteswap 16-bit
 *
 * @param value to byteswap
 *
 * @return byteswapped value
 */
ALWAYS_INLINE CONST u16
bswap16(const u16 x)
/*@*/
{
#if HAS_BUILTIN(BUILTIN_BSWAP16)
	return (u16) BUILTIN_BSWAP16(x);
#elif HAS_BUILTIN(BUILTIN_BSWAP32)
	return (u16) BUILTIN_BSWAP32(((u32) x) << 16u);
#elif HAS_BUILTIN(BUILTIN_BSWAP64)
	return (u16) BUILTIN_BSWAP64(((u64) x) << 48u);
#else
	u16 r = 0;
	r |= (x & 0xFF00u) >> 8u;
	r |= (x & 0x00FFu) << 8u;
	return r;
#endif
}

/**@fn bswap32
 * @brief byteswap 32-bit
 *
 * @param value to byteswap
 *
 * @return byteswapped value
 */
ALWAYS_INLINE CONST u32
bswap32(const u32 x)
/*@*/
{
#if HAS_BUILTIN(BUILTIN_BSWAP32)
	return (u32) BUILTIN_BSWAP32(x);
#elif HAS_BUILTIN(BUILTIN_BSWAP64)
	return (u32) BUILTIN_BSWAP64(((u64) x) << 32u);
#else
	u32 r = 0;
	r |= (x & 0xFF000000u) >> 24u;
	r |= (x & 0x00FF0000u) >>  8u;
	r |= (x & 0x0000FF00u) <<  8u;
	r |= (x & 0x000000FFu) << 24u;
	return r;
#endif
}

/**@fn bswap64
 * @brief byteswap 64-bit
 *
 * @param value to byteswap
 *
 * @return byteswapped value
 */
ALWAYS_INLINE CONST u64
bswap64(const u64 x)
/*@*/
{
#if HAS_BUILTIN(BUILTIN_BSWAP64)
	return (u64) BUILTIN_BSWAP64(x);
#else
	u64 r = 0;
	r |= (x & 0xFF00000000000000u) >> 56u;
	r |= (x & 0x00FF000000000000u) >> 40u;
	r |= (x & 0x0000FF0000000000u) >> 24u;
	r |= (x & 0x000000FF00000000u) >>  8u;
	r |= (x & 0x00000000FF000000u) <<  8u;
	r |= (x & 0x0000000000FF0000u) << 24u;
	r |= (x & 0x000000000000FF00u) << 40u;
	r |= (x & 0x00000000000000FFu) << 56u;
	return r;
#endif
}

//==========================================================================//

#ifndef S_SPLINT_S
#ifndef __BYTE_ORDER__
#error "'__BYTE_ORDER__' not defined"
#endif
#ifndef __ORDER_BIG_ENDIAN__
#error "'__ORDER_BIG_ENDIAN__' not defined"
#endif
#ifndef __ORDER_LITTLE_ENDIAN__
#error "'__ORDER_LITTLE_ENDIAN__' not defined"
#endif
#endif // S_SPLINT_S

/**@fn htole16
 * @brief host to little-endian 16-bit
 *
 * @param x value to conditionally byteswap
 *
 * @return little-endian value
**/
ALWAYS_INLINE CONST u16
htole16(const u16 x)
/*@*/
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	return bswap16(x);
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return x;
#else
#error "weird endianness"
#endif
}

 /**@fn letoh16
  * @brief little-endian to host 16-bit
  *
  * @param x value to conditionally byteswap
  *
  * @return host-endian value
 **/
ALWAYS_INLINE CONST u16
letoh16(const u16 x)
/*@*/
{
	return htole16(x);
}

 /**@fn htole32
  * @brief host to little-endian 32-bit
  *
  * @param x value to conditionally byteswap
  *
  * @return little-endian value
 **/
ALWAYS_INLINE CONST u32
htole32(const u32 x)
/*@*/
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	return bswap32(x);
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return x;
#else
#error "weird endianness"
#endif
}

 /**@fn letoh32
  * @brief little-endian to host 32-bit
  *
  * @param x value to conditionally byteswap
  *
  * @return host-endian value
 **/

ALWAYS_INLINE CONST u32
letoh32(const u32 x)
/*@*/
{
	return htole32(x);
}

 /**@fn htole64
  * @brief host to little-endian 64-bit
  *
  * @param x value to conditionally byteswap
  *
  * @return little-endian value
 **/
ALWAYS_INLINE CONST u64
htole64(const u64 x)
/*@*/
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
	return bswap64(x);
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	return x;
#else
#error "weird endianness"
#endif
}

 /**@fn letoh64
  * @brief little-endian to host 64-bit
  *
  * @param x value to conditionally byteswap
  *
  * @return host-endian value
 **/

ALWAYS_INLINE CONST u64
letoh64(const u64 x)
/*@*/
{
	return htole64(x);
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
