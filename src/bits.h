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

#include <stdint.h>

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

//////////////////////////////////////////////////////////////////////////////

#define INLINE		/*@unused@*/ static inline
#define ALWAYS_INLINE	INLINE __attribute__((always_inline))
#define NOINLINE	static __attribute__((noinline))

#define HOT		__attribute__((hot))
#define COLD		__attribute__((cold))

#define PURE		__attribute__((pure))
#define CONST		__attribute__((const))

#define NORETURN	/*@noreturn@*/ __attribute__((noreturn))
#define UNUSED		/*@unused@*/   __attribute__((unused))

#define PACKED		__attribute__((packed))
#define HIDDEN		__attribute__((visibility("hidden")))

//==========================================================================//

#ifdef __has_builtin
#define HAS_BUILTIN(x)	__has_builtin(x)
#else
#define HAS_BUILTIN(x)	0
#endif

//--------------------------------------------------------------------------//

// these should always be A/B tested; not always beneficial

#if HAS_BUILTIN(__builtin_expect)
#define LIKELY(cond)		(__builtin_expect((cond), true))
#define UNLIKELY(cond)		(__builtin_expect((cond), false))
#else
#define LIKELY(cond)		(cond)
#define UNLIKELY(cond)		(cond)
#endif

#if HAS_BUILTIN(__builtin_expect_with_probability)
#define PROBABLE(cond, prob)	( \
	__builtin_expect_with_probability((cond), true, (prob)) \
)
#define IMPROBABLE(cond, prob)	( \
	__builtin_expect_with_probability((cond), false, 1.0 - (prob)) \
)
#else
#define PROBABLE(cond, prob)	(cond)
#define IMPROBABLE(cond, prob)	(cond)
#endif

//--------------------------------------------------------------------------//

// mainly for byte-shaving errors

#if HAS_BUILTIN(__builtin_unreachable)
#define UNREACHABLE		__builtin_unreachable(); /*@notreached@*/
#else
#define UNREACHABLE		; /*@notreached@*/
#endif

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

typedef uint_fast64_t	u64fast;

//////////////////////////////////////////////////////////////////////////////

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

#define BUILTIN_BSWAP16		__builtin_bswap16
#define BUILTIN_BSWAP32		__builtin_bswap32
#define BUILTIN_BSWAP64		__builtin_bswap64

//////////////////////////////////////////////////////////////////////////////

/**@fn bswap16
 * @brief byteswap 16-bit
 *
 * @param value to byteswap
 *
 * @return byteswapped value
 */
ALWAYS_INLINE CONST u16
bswap16(register const u16 x)
/*@*/
{
#if HAS_BUILTIN(BUILTIN_BSWAP16)
	return (u16) BUILTIN_BSWAP16(x);
#elif HAS_BUILTIN(BUILTIN_BSWAP32)
	return (u16) BUILTIN_BSWAP32(((u32) x) << 16u);
#elif HAS_BUILTIN(BUILTIN_BSWAP64)
	return (u16) BUILTIN_BSWAP64(((u64) x) << 48u);
#else
	register u16 r = 0;
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
bswap32(register const u32 x)
/*@*/
{
#if HAS_BUILTIN(BUILTIN_BSWAP32)
	return (u32) BUILTIN_BSWAP32(x);
#elif HAS_BUILTIN(BUILTIN_BSWAP64)
	return (u32) BUILTIN_BSWAP64(((u64) x) << 32u);
#else
	register u32 r = 0;
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
bswap64(register const u64 x)
/*@*/
{
#if HAS_BUILTIN(BUILTIN_BSWAP64)
	return (u64) BUILTIN_BSWAP64(x);
#else
	register u64 r = 0;
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

/**@fn htole16
 * @brief host to little-endian 16-bit
 *
 * @param x value to conditionally byteswap
 *
 * @return little-endian value
**/
ALWAYS_INLINE CONST u16
htole16(register const u16 x)
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
letoh16(register const u16 x)
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
htole32(register const u32 x)
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
letoh32(register const u32 x)
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
htole64(register const u64 x)
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
letoh64(register const u64 x)
/*@*/
{
	return htole64(x);
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
