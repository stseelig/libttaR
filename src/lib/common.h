#ifndef TTA_CODEC_COMMON_H
#define TTA_CODEC_COMMON_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/common.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <limits.h>
#include <stddef.h>	// size_t
#include <string.h>	// memmove, memset

#include "../bits.h"
#include "../splint.h"

//////////////////////////////////////////////////////////////////////////////

#ifdef LIBTTAr_OPT_SLOW_CPU

#define LIBTTAr_OPT_NO_FAT_RICE_ENCODER
#define LIBTTAr_OPT_NO_NATIVE_TZCNT
#define LIBTTAr_OPT_ONLY_NECESSARY_FAST_TYPES
#define LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
#define LIBTTAr_OPT_PREFER_LOOKUP_TABLES

#else // ! defined(LIBTTAr_OPT_SLOW_CPU)

//#define LIBTTAr_OPT_NO_FAT_RICE_ENCODER
//#define LIBTTAr_OPT_NO_NATIVE_TZCNT
//#define LIBTTAr_OPT_ONLY_NECESSARY_FAST_TYPES
//#define LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
//#define LIBTTAr_OPT_PREFER_LOOKUP_TABLES

#endif // LIBTTAr_OPT_SLOW_CPU

//--------------------------------------------------------------------------//

// this tested much slower even when NO_FAT_RICE_ENCODER was faster
//   - Intel Celeron N2830, clang 11

//#define LIBTTAr_OPT_NO_FAT_RICE_DECODER";

//////////////////////////////////////////////////////////////////////////////

enum TTASampleBytes {
	TTASAMPLEBYTES_1 = 1u,
	TTASAMPLEBYTES_2 = 2u,
	TTASAMPLEBYTES_3 = 3u
};
#define TTA_SAMPLEBYTES_MAX	((uint) TTASAMPLEBYTES_3)
#define TTA_SAMPLEBITS_MAX	((uint) (8u * TTA_SAMPLEBYTES_MAX))

//////////////////////////////////////////////////////////////////////////////

// fast types specifically tuned for the AMD Ryzen 7 1700 & clang 11
#ifndef LIBTTAr_OPT_ONLY_NECESSARY_FAST_TYPES
typedef u32	rice24_enc;
typedef u32f	rice24_dec;
typedef u32	crc32_enc;
typedef u32f	crc32_dec;
typedef u32f	cache32;
typedef u64f	cache64;
typedef  u8f	bitcnt;
#else // defined(LIBTTAr_OPT_ONLY_NECESSARY_FAST_TYPES)
typedef u32	rice24_enc;
typedef u32	rice24_dec;
typedef u32	crc32_enc;
typedef u32	crc32_dec;
typedef u32	cache32;
typedef u64f	cache64;
typedef  u8	bitcnt;
#endif // LIBTTAr_OPT_ONLY_NECESSARY_FAST_TYPES

//==========================================================================//

struct BitCache_Enc {
	cache64	cache;
	bitcnt	count;
};

struct BitCache_Dec {
	cache32	cache;
	bitcnt	count;
};

union BitCache {
	struct BitCache_Enc enc;
	struct BitCache_Dec dec;
};

struct Rice {
	u32	sum[2u];	// needs 32-bit wrapping
	bitcnt	k[2u];
};

struct Filter {
	i32	qm[8u];
	i32	dx[9u];		// the extra value is for a memmove trick
	i32	dl[9u];		// ~
	i32	error;		// sign of the error (-1, 1, or 0)
};

struct Codec {
	struct Filter	filter;
	struct Rice	rice;
	i32		prev;
};

struct LibTTAr_CodecState_Priv {
	union BitCache	bitcache;
	struct Codec 	codec[];
};

struct LibTTAr_CodecState_User {
	u32	ncalls_codec;
	u32	crc;
	size_t	ni32;			// enc: num read, dec: num written
	size_t	ni32_total;		// ~
	size_t	nbytes_tta;		// enc: num written, dec: num read
	size_t	nbytes_tta_total;	// ~
};

//////////////////////////////////////////////////////////////////////////////

#undef priv
INLINE void state_priv_init(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	const uint
)
/*@modifies	*priv@*/
;

#undef codec
INLINE void codec_init(
	/*@out@*/ struct Codec *const restrict codec, const uint
)
/*@modifies	*codec@*/
;

#undef rice
INLINE void rice_init(/*@out@*/ struct Rice *const restrict rice)
/*@modifies	*rice@*/
;

//////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

#if UINT_MAX == UINT32_MAX
#define BUILTIN_TZCNT32			__builtin_ctz
#elif ULONG_MAX == UINT32_MAX
#define BUILTIN_TZCNT32			__builtin_ctzl
#else
#define BUILTIN_TZCNT32			0
#endif

#if UINT_MAX == UINT64_MAX
#define BUILTIN_TZCNT64			__builtin_ctz
#elif ULONG_MAX == UINT64_MAX
#define BUILTIN_TZCNT64			__builtin_ctzl
#elif ULONG_LONG_MAX == UINT64_MAX
#define BUILTIN_TZCNT64			__builtin_ctzll
#else
#define BUILTIN_TZCNT64			0
#endif

// the _inline's cause clang to panic (Debian clang version 11.0.1-2)
//#define BUILTIN_MEMCPY_INLINE		__builtin_memcpy_inline
#define BUILTIN_MEMCPY			__builtin_memcpy
//#define BUILTIN_MEMMOVE_INLINE	__builtin_memmove_inline
#define BUILTIN_MEMMOVE			__builtin_memmove
//#define BUILTIN_MEMSET_INLINE		__builtin_memset_inline
#define BUILTIN_MEMSET			__builtin_memset

#else // ! defined(__GNUC__)

#define BUILTIN_TZCNT32			0
#define BUILTIN_TZCNT64			0

#define BUILTIN_MEMCPY_INLINE		0
#define BUILTIN_MEMCPY			0
#define BUILTIN_MEMMOVE_INLINE		0
#define BUILTIN_MEMMOVE			0
#define BUILTIN_MEMSET_INLINE		0
#define BUILTIN_MEMSET			0

#endif // __GNUC__


//////////////////////////////////////////////////////////////////////////////

#if (! defined(LIBTTAr_OPT_PREFER_LOOKUP_TABLES)) \
&&  (! defined(LIBTTAr_OPT_NO_NATIVE_TZCNT))
#if (! HAS_BUILTIN(BUILTIN_TZCNT32)) && (! HAS_BUILTIN(BUILTIN_TZCNT64))
#pragma message "compiler does not have a builtin 'tzcnt'"
#endif
#endif

#if defined(LIBTTAr_OPT_NO_NATIVE_TZCNT) \
 || defined(LIBTTAr_OPT_PREFER_LOOKUP_TABLES) \
 || ((! HAS_BUILTIN(BUILTIN_TZCNT32)) && (! HAS_BUILTIN(BUILTIN_TZCNT64)))
#define TBCNT8_TABLE
#endif

//==========================================================================//

// -nolibc; a decent compiler should do this anyway

#if HAS_BUILTIN(BUILTIN_MEMCPY_INLINE)
#define MEMCPY(dest, src, n)	BUILTIN_MEMCPY_INLINE((dest), (src), (n))
#elif HAS_BUILTIN(BUILTIN_MEMCPY)
#define MEMCPY(dest, src, n)	BUILTIN_MEMCPY((dest), (src), (n))
#else
#pragma message "compiler does not have a builtin 'memcpy'"
#define MEMCPY(dest, src, n)	((void) memcpy((dest), (src), (n)))
#endif

#if HAS_BUILTIN(BUILTIN_MEMMOVE_INLINE)
#define MEMMOVE(dest, src, n)	BUILTIN_MEMMOVE_INLINE((dest), (src), (n))
#elif HAS_BUILTIN(BUILTIN_MEMMOVE)
#define MEMMOVE(dest, src, n)	BUILTIN_MEMMOVE((dest), (src), (n))
#else
// gcc will not reach here, even though it does not have a builtin memmove
#pragma message "compiler does not have a builtin 'memmove'"
#define MEMMOVE(dest, src, n)	((void) memmove((dest), (src), (n)))
#endif

#if HAS_BUILTIN(BUILTIN_MEMSET_INLINE)
#define MEMSET(s, c, n)		BUILTIN_MEMSET_INLINE((s), (c), (n))
#elif HAS_BUILTIN(BUILTIN_MEMSET)
#define MEMSET(s, c, n)		BUILTIN_MEMSET((s), (c), (n))
#else
#pragma message "compiler does not have a builtin 'memset'"
#define MEMSET(s, c, n)		memset((s), (c), (n))
#endif

//////////////////////////////////////////////////////////////////////////////

/**@fn state_priv_init
 * @brief initializes a private state struct
 *
 * @param priv[out] the private state struct
 * @param nchan number of audio channels
**/
INLINE void
state_priv_init(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	const uint nchan
)
/*@modifies	*priv@*/
{
	MEMSET(&priv->bitcache, 0x00, sizeof priv->bitcache);
	codec_init((struct Codec *) &priv->codec, nchan);
	return;
}

//--------------------------------------------------------------------------//

/**@fn codec_init
 * @brief initializes an array of 'struct Codec'
 *
 * @param codec[out] the struct array to initialize
 * @param nchan number of audio channels
**/
INLINE void
codec_init(
	/*@out@*/ struct Codec *const restrict codec, const uint nchan
)
/*@modifies	*codec@*/
{
	uint i;
	for ( i = 0; i < nchan; ++i ){
		MEMSET(&codec[i].filter, 0x00, sizeof codec[i].filter);
		rice_init(&codec[i].rice);
		codec[i].prev = 0;
	}
	return;
}

/**@fn rice_init
 * @brief initializes a 'struct Rice'
 *
 * @param rice[out] struct to initialize
**/
INLINE void
rice_init(/*@out@*/ struct Rice *const restrict rice)
/*@modifies	*rice@*/
{
	rice->sum[0u] = (u32) 0x00004000u;	// binexp32p4((bitcnt) 10u)
	rice->sum[1u] = (u32) 0x00004000u;	// ~
	rice->k[0u]   = (bitcnt) 10u;
	rice->k[1u]   = (bitcnt) 10u;
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
