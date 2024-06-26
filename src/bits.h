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

#include <limits.h>
#include <stdbool.h>	// true, false
#include <stdint.h>
#include <string.h>	// memmove, memset

#include "splint.h"

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

#undef asl32
#undef asr32

#undef tzcnt32
#undef tbcnt32

//////////////////////////////////////////////////////////////////////////////

#define INLINE		/*@unused@*/ static inline
#define ALWAYS_INLINE	INLINE __attribute__((always_inline))
#define NOINLINE	static __attribute__((noinline))

#define HOT		__attribute__((hot))
#define COLD		__attribute__((cold))

#define PURE		__attribute__((pure))
#define CONST		__attribute__((const))

#define NORETURN	/*@noreturn@*/ __attribute__((noreturn))
#define UNUSED		/*@unused@*/ __attribute__((unused))

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
#define LIKELY(cond)		(__builtin_expect(!!(cond), true))
#define UNLIKELY(cond)		(__builtin_expect(!!(cond), false))
#else
#define LIKELY(cond)		(cond)
#define UNLIKELY(cond)		(cond)
#endif

#if HAS_BUILTIN(__builtin_expect_with_probability)
#define LIKELY_P(cond, prob)	( \
	__builtin_expect_with_probability(!!(cond), true, (prob)) \
)
#define UNLIKELY_P(cond, prob)	( \
	__builtin_expect_with_probability(!!(cond), false, (prob)) \
)
#else
#define LIKELY_P(cond, prob)	(cond)
#define UNLIKELY_P(cond, prob)	(cond)
#endif

//--------------------------------------------------------------------------//

// mainly for byte-shaving errors

#if HAS_BUILTIN(__builtin_unreachable)
#define UNREACHABLE		__builtin_unreachable(); /*@notreached@*/
#else
#define UNREACHABLE		; /*@notreached@*/
#endif

//--------------------------------------------------------------------------//

// -nolibc for library; a decent compiler should do this anyway

#if HAS_BUILTIN(__builtin_memmove)
#define MEMMOVE(dest, src, n)	((void) __builtin_memmove((dest), (src), (n)))
#else
#define MEMMOVE(dest, src, n)	((void) memmove((dest), (src), (n)))
#endif

#if HAS_BUILTIN(__builtin_memset)
#define MEMSET(s, c, n)		(__builtin_memset((s), (c), (n)))
#else
#define MEMSET(s, c, n)		(memset((s), (c), (n)))
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

ALWAYS_INLINE CONST i32 asl32(i32, u8) /*@*/;
ALWAYS_INLINE CONST i32 asr32(i32, u8) /*@*/;

ALWAYS_INLINE CONST uint tzcnt32(u32) /*@*/;
ALWAYS_INLINE CONST uint tbcnt32(u32) /*@*/;

//////////////////////////////////////////////////////////////////////////////

#define BUILTIN_BSWAP16			__builtin_bswap16
#define BUILTIN_BSWAP32			__builtin_bswap32
#define BUILTIN_BSWAP64			__builtin_bswap64

//--------------------------------------------------------------------------//

#define HAS_ASR(type) ( \
	/*@-shiftimplementation@*/ \
	((type) (((type) (-1)) >> 1u)) == ((type) (-1)) \
	/*@=shiftimplementation@*/ \
)

//--------------------------------------------------------------------------//

#ifndef S_SPLINT_S	// splint preproc bugs

#if UINT_MAX == UINT32_MAX
#define BUILTIN_TZCNT32			__builtin_ctz
#elif ULONG_MAX == UINT32_MAX
#define BUILTIN_TZCNT32			__builtin_ctzl
#else
#define BUILTIN_TZCNT32			nil
#endif

#if UINT_MAX == UINT64_MAX
#define BUILTIN_TZCNT64			__builtin_ctz
#elif ULONG_MAX == UINT64_MAX
#define BUILTIN_TZCNT64			__builtin_ctzl
#elif ULONG_LONG_MAX == UINT64_MAX
#define BUILTIN_TZCNT64			__builtin_ctzll
#else
#define BUILTIN_TZCNT64			nil
#endif

#endif

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

//==========================================================================//

// shifting signed integers is naughty (implementation defined)

/**@fn asl32
 * @brief arithmetic shift left 32-bit
 *
 * @param x value to shift
 * @param k amount to shift
 *
 * @return shifted value
**/
ALWAYS_INLINE CONST i32
asl32(register const i32 x, register const u8 k)
/*@*/
{
	return (i32) (((u32) x) << k);
}

/**@fn asr32
 * @brief arithmetic shift right 32-bit
 *
 * @param x value to shift
 * @param k amount to shift
 *
 * @return shifted value
**/
ALWAYS_INLINE CONST i32
asr32(register const i32 x, register const u8 k)
/*@*/
{
	if ( (! HAS_ASR(i32)) && (x < 0) ){
		return (i32) ~((~((u32) x)) >> k);
	}
	else {	/*@-shiftimplementation@*/
		return (i32) (x >> k);
		/*@=shiftimplementation@*/
	}
}

//==========================================================================//

/**@fn tzcnt32
 * @brief trailing zero count 32-bit
 *
 * @param x value to count
 *
 * @return number of trailing zeroes
 *
 * @note undefined for 0
**/
ALWAYS_INLINE CONST uint
tzcnt32(register const u32 x)
/*@*/
{
#if HAS_BUILTIN(BUILTIN_TZCNT32)
	return (uint) BUILTIN_TZCNT32(x);
#elif HAS_BUILTIN(BUILTIN_TZCNT64)
	return (uint) BUILTIN_TZCNT64((u64) x);
#else
	// https://graphics.stanford.edu/~seander/bithacks.html
	register uint r = 0;
	if ( (x & 0x1u) == 0 ){
		r = 1u;
		if ( (x & 0xFFFFu) == 0 ){ r |= 16u, x >>= 16u; }
		if ( (x & 0x00FFu) == 0 ){ r |=  8u, x >>=  8u; }
		if ( (x & 0x000Fu) == 0 ){ r |=  4u, x >>=  4u; }
		if ( (x & 0x0003u) == 0 ){ r |=  2u, x >>=  2u; }
		r -=  x & 0x0001u;
	}
	return r;
#endif
}

/**@fn tbcnt32
 * @brief trailing bit count 32-bit
 *
 * @param x value to count
 *
 * @return number of trailing bits
 *
 * @note undefined for UINT32_MAX
**/
ALWAYS_INLINE CONST uint
tbcnt32(register const u32 x)
/*@*/
{
	return tzcnt32(~x);
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
