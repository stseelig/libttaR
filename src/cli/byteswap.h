#ifndef H_TTA_BYTESWAP_H
#define H_TTA_BYTESWAP_H
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

#include <stdint.h>

#include "./common.h"

/* ======================================================================== */

#ifndef S_SPLINT_S

#ifndef __BYTE_ORDER__
#error "__BYTE_ORDER__"
#endif	/* __BYTE_ORDER__ */

#ifndef __ORDER_BIG_ENDIAN__
#error "__ORDER_BIG_ENDIAN__"
#endif	/* __ORDER_BIG_ENDIAN__ */

#ifndef __ORDER_LITTLE_ENDIAN__
#error "__ORDER_LITTLE_ENDIAN__"
#endif	/* __ORDER_LITTLE_ENDIAN__ */

#endif /* S_SPLINT_S */

/* //////////////////////////////////////////////////////////////////////// */

#ifdef __GNUC__

#define X_BUILTIN_GNUC_BSWAP16		__builtin_bswap16
#define X_BUILTIN_GNUC_BSWAP32		__builtin_bswap32
#define X_BUILTIN_GNUC_BSWAP64		__builtin_bswap64

#else /* !defined(__GNUC__) */

#define X_BUILTIN_GNUC_BSWAP16		nil
#define X_BUILTIN_GNUC_BSWAP32		nil
#define X_BUILTIN_GNUC_BSWAP64		nil

#endif /* __GNUC__ */

/* ======================================================================== */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_BSWAP16)
#define	X_BUILTIN_BSWAP16(x_x)	X_BUILTIN_GNUC_BSWAP16(x_x)
#define HAS_BUILTIN_BSWAP16
#endif	/* BUILTIN_BSWAP16 */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_BSWAP32)
#define	X_BUILTIN_BSWAP32(x_x)	X_BUILTIN_GNUC_BSWAP32(x_x)
#define HAS_BUILTIN_BSWAP32
#endif	/* BUILTIN_BSWAP32 */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_BSWAP64)
#define	X_BUILTIN_BSWAP64(x_x)	X_BUILTIN_GNUC_BSWAP64(x_x)
#define HAS_BUILTIN_BSWAP64
#endif	/* BUILTIN_BSWAP64 */

/* //////////////////////////////////////////////////////////////////////// */

CONST
ALWAYS_INLINE uint16_t byteswap_u16(uint16_t) /*@*/;

CONST
ALWAYS_INLINE uint32_t byteswap_u32(uint32_t) /*@*/;

CONST
ALWAYS_INLINE uint64_t byteswap_u64(uint64_t) /*@*/;

/* ------------------------------------------------------------------------ */

CONST
ALWAYS_INLINE uint16_t byteswap_htole_u16(uint16_t) /*@*/;

CONST
ALWAYS_INLINE uint16_t byteswap_letoh_u16(uint16_t) /*@*/;

CONST
ALWAYS_INLINE uint32_t byteswap_htole_u32(uint32_t) /*@*/;

CONST
ALWAYS_INLINE uint32_t byteswap_letoh_u32(uint32_t) /*@*/;

CONST
ALWAYS_INLINE uint64_t byteswap_htole_u64(uint64_t) /*@*/;

CONST
ALWAYS_INLINE uint64_t byteswap_letoh_u64(uint64_t) /*@*/;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn byteswap_u16
 * @brief byteswap 16-bit
 *
 * @param value to byteswap
 *
 * @return byteswapped value
**/
CONST
ALWAYS_INLINE uint16_t
byteswap_u16(const uint16_t x)
/*@*/
{
#if defined(HAS_BUILTIN_BSWAP16)

	return (uint16_t) X_BUILTIN_BSWAP16(x);

#elif defined(HAS_BUILTIN_BSWAP32)

	return (uint16_t) X_BUILTIN_BSWAP32(((uint32_t) x) << 16u);

#elif defined(HAS_BUILTIN_BSWAP64)

	return (uint16_t) X_BUILTIN_BSWAP64(((uint64_t) x) << 48u);

#else

	uint16_t retval = 0;

	retval |= (x & UINT16_C(0xFF00)) >> 8u;
	retval |= (x & UINT16_C(0x00FF)) << 8u;

	return retval;

#endif
}

/**@fn byteswap_u32
 * @brief byteswap 32-bit
 *
 * @param value to byteswap
 *
 * @return byteswapped value
**/
CONST
ALWAYS_INLINE uint32_t
byteswap_u32(const uint32_t x)
/*@*/
{
#if defined(HAS_BUILTIN_BSWAP32)

	return (uint32_t) X_BUILTIN_BSWAP32(x);

#elif defined(HAS_BUILTIN_BSWAP64)

	return (uint32_t) X_BUILTIN_BSWAP64(((uint64_t) x) << 32u);

#else

	uint32_t retval = 0;

	retval |= (x & UINT32_C(0xFF000000)) >> 24u;
	retval |= (x & UINT32_C(0x00FF0000)) >>  8u;
	retval |= (x & UINT32_C(0x0000FF00)) <<  8u;
	retval |= (x & UINT32_C(0x000000FF)) << 24u;

	return retval;

#endif
}

/**@fn byteswap_u64
 * @brief byteswap 64-bit
 *
 * @param value to byteswap
 *
 * @return byteswapped value
**/
CONST
ALWAYS_INLINE uint64_t
byteswap_u64(const uint64_t x)
/*@*/
{
#if defined(HAS_BUILTIN_BSWAP64)

	return (uint64_t) X_BUILTIN_BSWAP64(x);

#else

	uint64_t retval = 0;

	retval |= (x & UINT64_C(0xFF00000000000000)) >> 56u;
	retval |= (x & UINT64_C(0x00FF000000000000)) >> 40u;
	retval |= (x & UINT64_C(0x0000FF0000000000)) >> 24u;
	retval |= (x & UINT64_C(0x000000FF00000000)) >>  8u;
	retval |= (x & UINT64_C(0x00000000FF000000)) <<  8u;
	retval |= (x & UINT64_C(0x0000000000FF0000)) << 24u;
	retval |= (x & UINT64_C(0x000000000000FF00)) << 40u;
	retval |= (x & UINT64_C(0x00000000000000FF)) << 56u;

	return retval;

#endif
}

/* ======================================================================== */

/**@fn byteswap_htole_u16
 * @brief host to little-endian 16-bit
 *
 * @param x - value to conditionally byteswap
 *
 * @return little-endian value
**/
CONST
ALWAYS_INLINE uint16_t
byteswap_htole_u16(const uint16_t x)
/*@*/
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

	return byteswap_u16(x);

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	return x;

#else
#error "__BYTE_ORDER__"
#endif
}

/**@fn byteswap_letoh_u16
* @brief little-endian to host 16-bit
*
* @param x - value to conditionally byteswap
*
* @return host-endian value
**/
CONST
ALWAYS_INLINE uint16_t
byteswap_letoh_u16(const uint16_t x)
/*@*/
{
	return byteswap_htole_u16(x);
}

/**@fn byteswap_htole_u32
* @brief host to little-endian 32-bit
*
* @param x - value to conditionally byteswap
*
* @return little-endian value
**/
CONST
ALWAYS_INLINE uint32_t
byteswap_htole_u32(const uint32_t x)
/*@*/
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

	return byteswap_u32(x);

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	return x;

#else
#error "__BYTE_ORDER__"
#endif
}

/**@fn byteswap_letoh_u32
* @brief little-endian to host 32-bit
*
* @param x - value to conditionally byteswap
*
* @return host-endian value
**/
CONST
ALWAYS_INLINE uint32_t
byteswap_letoh_u32(const uint32_t x)
/*@*/
{
	return byteswap_htole_u32(x);
}

/**@fn byteswap_htole_u64
* @brief host to little-endian 64-bit
*
* @param x - value to conditionally byteswap
*
* @return little-endian value
**/
CONST
ALWAYS_INLINE uint64_t
byteswap_htole_u64(const uint64_t x)
/*@*/
{
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

	return byteswap_u64(x);

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

	return x;

#else
#error "__BYTE_ORDER__"
#endif
}

/**@fn byteswap_letoh_u64
* @brief little-endian to host 64-bit
*
* @param x - value to conditionally byteswap
*
* @return host-endian value
**/
CONST
ALWAYS_INLINE uint64_t
byteswap_letoh_u64(const uint64_t x)
/*@*/
{
	return byteswap_htole_u64(x);
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_BYTESWAP_H */
