#ifndef TTA_CODEC_COMMON_H
#define TTA_CODEC_COMMON_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/common.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef S_SPLINT_S
#include "../splint.h"
#endif

/* ------------------------------------------------------------------------ */

#include <limits.h>
#include <stddef.h>	// size_t
#include <string.h>	// memmove, memset

#include "../bits.h"

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

//#define LIBTTAr_OPT_NO_FAT_RICE_DECODER;

//////////////////////////////////////////////////////////////////////////////

#ifdef __GNUC__

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

// the _inline's cause clang to panic (Debian clang version 11.0.1-2)
//#define BUILTIN_MEMMOVE_INLINE	__builtin_memmove_inline
#define BUILTIN_MEMMOVE			__builtin_memmove
//#define BUILTIN_MEMSET_INLINE		__builtin_memset_inline
#define BUILTIN_MEMSET			__builtin_memset

#else // ! defined(__GNUC__)

#define BUILTIN_TZCNT32			nil
#define BUILTIN_TZCNT64			nil

#define BUILTIN_MEMMOVE_INLINE		nil
#define BUILTIN_MEMMOVE			nil
#define BUILTIN_MEMSET_INLINE		nil
#define BUILTIN_MEMSET			nil

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
typedef u8f		bitcnt;
typedef u32		rice24_enc;
typedef u32f		rice24_dec;
typedef u32		crc32_enc;
typedef u32f		crc32_dec;
typedef u32f		cache32;
typedef u64f		cache64;
typedef rice24_enc	bitcnt_enc;
typedef bitcnt		bitcnt_dec;
#else // defined(LIBTTAr_OPT_ONLY_NECESSARY_FAST_TYPES)
typedef u8		bitcnt;
typedef u32		rice24_enc;
typedef u32		rice24_dec;
typedef u32		crc32_enc;
typedef u32		crc32_dec;
typedef u32		cache32;
typedef u64f		cache64;
typedef rice24_enc	bitcnt_enc;
typedef bitcnt		bitcnt_dec;
#endif // LIBTTAr_OPT_ONLY_NECESSARY_FAST_TYPES

//==========================================================================//

struct BitCache_Enc {
	cache64		cache;
	bitcnt_enc	count;
};

struct BitCache_Dec {
	cache32		cache;
	bitcnt_dec	count;
};

union BitCache {
	struct BitCache_Enc	enc;
	struct BitCache_Dec	dec;
};

struct Rice_Enc {
	u32		sum[2u];	// needs 32-bit wrapping
	bitcnt_enc	k[2u];
};

struct Rice_Dec {
	u32		sum[2u];	// needs 32-bit wrapping
	bitcnt_dec	k[2u];
};

union Rice {
	struct Rice_Enc	enc;
	struct Rice_Dec	dec;
};

// struct Filter
#include "filter.h"

struct ALIGNED(16u) Codec {	// alignment necessary for intrinsics (filter)
	struct Filter	filter;
	union Rice	rice;
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
INLINE void state_priv_init_enc(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	const uint
)
/*@modifies	*priv@*/
;

#undef priv
INLINE void state_priv_init_dec(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	const uint
)
/*@modifies	*priv@*/
;

#undef codec
INLINE void codec_init_enc(
	/*@out@*/ struct Codec *const restrict codec, const uint
)
/*@modifies	*codec@*/
;

#undef codec
INLINE void codec_init_dec(
	/*@out@*/ struct Codec *const restrict codec, const uint
)
/*@modifies	*codec@*/
;

#undef rice
INLINE void rice_init_enc(/*@out@*/ struct Rice_Enc *const restrict rice)
/*@modifies	*rice@*/
;

#undef rice
INLINE void rice_init_dec(/*@out@*/ struct Rice_Dec *const restrict rice)
/*@modifies	*rice@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn state_priv_init_enc
 * @brief initializes a private state struct; encode version
 *
 * @param priv[out] the private state struct
 * @param nchan number of audio channels
**/
INLINE void
state_priv_init_enc(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	const uint nchan
)
/*@modifies	*priv@*/
{
	MEMSET(&priv->bitcache, 0x00, sizeof priv->bitcache);
	codec_init_enc((struct Codec *) &priv->codec, nchan);
	return;
}

/**@fn state_priv_init_dec
 * @brief initializes a private state struct; decode version
 *
 * @param priv[out] the private state struct
 * @param nchan number of audio channels
**/
INLINE void
state_priv_init_dec(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	const uint nchan
)
/*@modifies	*priv@*/
{
	MEMSET(&priv->bitcache, 0x00, sizeof priv->bitcache);
	codec_init_dec((struct Codec *) &priv->codec, nchan);
	return;
}

//--------------------------------------------------------------------------//

/**@fn codec_init_enc
 * @brief initializes an array of 'struct Codec'; encode version
 *
 * @param codec[out] the struct array to initialize
 * @param nchan number of audio channels
**/
INLINE void
codec_init_enc(
	/*@out@*/ struct Codec *const restrict codec, const uint nchan
)
/*@modifies	*codec@*/
{
	uint i;
	for ( i = 0; i < nchan; ++i ){
		MEMSET(&codec[i].filter, 0x00, sizeof codec[i].filter);
		rice_init_enc(&codec[i].rice.enc);
		codec[i].prev = 0;
	}
	return;
}

/**@fn codec_init_dec
 * @brief initializes an array of 'struct Codec'; decode version
 *
 * @param codec[out] the struct array to initialize
 * @param nchan number of audio channels
**/
INLINE void
codec_init_dec(
	/*@out@*/ struct Codec *const restrict codec, const uint nchan
)
/*@modifies	*codec@*/
{
	uint i;
	for ( i = 0; i < nchan; ++i ){
		MEMSET(&codec[i].filter, 0x00, sizeof codec[i].filter);
		rice_init_dec(&codec[i].rice.dec);
		codec[i].prev = 0;
	}
	return;
}

/**@fn rice_init
 * @brief initializes a 'struct Rice'; encode version
 *
 * @param rice[out] struct to initialize
**/
INLINE void
rice_init_enc(/*@out@*/ struct Rice_Enc *const restrict rice)
/*@modifies	*rice@*/
{
	rice->sum[0u] = (u32) 0x00004000u;	// binexp32p4((bitcnt) 10u)
	rice->sum[1u] = (u32) 0x00004000u;	// ~
	rice->k[0u]   = (bitcnt_enc) 10u;
	rice->k[1u]   = (bitcnt_enc) 10u;
	return;
}

/**@fn rice_init
 * @brief initializes a 'struct Rice'; decode version
 *
 * @param rice[out] struct to initialize
**/
INLINE void
rice_init_dec(/*@out@*/ struct Rice_Dec *const restrict rice)
/*@modifies	*rice@*/
{
	rice->sum[0u] = (u32) 0x00004000u;	// binexp32p4((bitcnt) 10u)
	rice->sum[1u] = (u32) 0x00004000u;	// ~
	rice->k[0u]   = (bitcnt_dec) 10u;
	rice->k[1u]   = (bitcnt_dec) 10u;
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
