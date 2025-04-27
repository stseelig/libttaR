#ifndef TTA_CODEC_RICE24_H
#define TTA_CODEC_RICE24_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/rice24.h                                                           //
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

#include <assert.h>
#include <limits.h>	// UINT32_MAX
#include <stddef.h>	// size_t

#include "../bits.h"

#include "common.h"
#include "crc32.h"

//////////////////////////////////////////////////////////////////////////////

#define UINT24_MAX		0xFFFFFFu

// the lax_limit has an extra byte to make handling invalid data faster/easier
#define UNARY_LAX_LIMIT_1_2	((rice24_dec) ((8u *    8194u)  - 1u))
#define UNARY_LAX_LIMIT_3	((rice24_dec) ((8u * 2097154uL) - 1u))

// max unary + binary r/w size for one value
#define RICE24_ENC_MAX_1_2	((size_t)    8199u)
#define RICE24_ENC_MAX_3	((size_t) 2097159uL)
#define RICE24_DEC_MAX_1_2	((size_t)    8197u)
#define RICE24_DEC_MAX_3	((size_t) 2097157uL)

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 binexp32p4_table[26u];

#ifdef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 lsmask32_table[32u];
#endif

#ifdef TBCNT8_TABLE
/*@unchecked@*/ /*@unused@*/
extern HIDDEN const bitcnt_dec tbcnt8_table[256u];
#endif

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE CONST
rice24_dec get_unary_lax_limit(enum LibTTAr_SampleBytes) /*@*/;

#ifndef NDEBUG
ALWAYS_INLINE CONST size_t get_rice24_enc_max(enum LibTTAr_SampleBytes) /*@*/;
ALWAYS_INLINE CONST size_t get_rice24_dec_max(enum LibTTAr_SampleBytes) /*@*/;
#endif

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST u32f binexp32(bitcnt) /*@*/;

#ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
ALWAYS_INLINE CONST u32f lsmask32(bitcnt) /*@*/;
#else
ALWAYS_INLINE CONST u32  lsmask32(bitcnt) /*@*/;
#endif	// #ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES

//--------------------------------------------------------------------------//

#undef crc
ALWAYS_INLINE u8 rice24_crc32_enc(u8, crc32_enc *restrict crc)
/*@modifies	*crc@*/
;

#undef crc
ALWAYS_INLINE u8 rice24_crc32_dec(u8, crc32_dec *restrict crc)
/*@modifies	*crc@*/
;

//--------------------------------------------------------------------------//

#undef sum
#undef k
ALWAYS_INLINE void rice24_update_enc(
	u32 *restrict sum, bitcnt_enc *restrict k, u32, const u32 *restrict
)
/*@modifies	*sum,
		*k
@*/
;

#undef sum
#undef k
ALWAYS_INLINE void rice24_update_dec(
	u32 *restrict sum, bitcnt_dec *restrict k, u32, const u32 *restrict
)
/*@modifies	*sum,
		*k
@*/
;

//--------------------------------------------------------------------------//

#undef dest
#undef rice
#undef bitcache
#undef crc
ALWAYS_INLINE size_t rice24_encode(
	/*@reldef@*/ u8 *restrict dest, u32, size_t,
	struct Rice_Enc *restrict rice,
	struct BitCache_Enc *restrict bitcache, crc32_enc *restrict crc
)
/*@modifies	*dest,
		*rice,
		*bitcache,
		*crc
@*/
;

#undef dest
#undef bitcache
#undef crc_inout
ALWAYS_INLINE size_t rice24_encode_cacheflush(
	/*@reldef@*/ u8 *restrict dest, size_t,
	struct BitCache_Enc *restrict bitcache, u32 *restrict crc_inout
)
/*@modifies	*dest,
		*bitcache,
		*crc_inout
@*/
;

#undef dest
#undef cache
#undef count
#undef crc
ALWAYS_INLINE size_t rice24_write_unary(
	/*@reldef@*/ u8 *restrict dest, rice24_enc, size_t,
	cache64 *restrict cache, bitcnt_enc *restrict count,
	crc32_enc *restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
;

#undef dest
#undef cache
#undef count
#undef crc
ALWAYS_INLINE size_t rice24_write_unary_zero(
	/*@reldef@*/ u8 *restrict dest, size_t, cache64 *restrict cache,
	bitcnt_enc *restrict count, crc32_enc *restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
;

#undef cache
#undef count
ALWAYS_INLINE void rice24_cache_binary(
	rice24_enc, bitcnt_enc, cache64 *restrict cache,
	bitcnt_enc *restrict count
)
/*@modifies	*cache,
		*count
@*/
;

#undef dest
#undef cache
#undef count
#undef crc
ALWAYS_INLINE size_t rice24_write_cache(
	/*@reldef@*/ u8 *restrict dest, size_t, cache64 *restrict cache,
	bitcnt_enc *restrict count, crc32_enc *restrict crc, bitcnt_enc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
;

//--------------------------------------------------------------------------//

#undef value
#undef rice
#undef bitcache
#undef crc
ALWAYS_INLINE size_t rice24_decode(
	/*@out@*/ u32 *restrict value, const u8 *restrict, size_t,
	struct Rice_Dec *restrict rice,
	struct BitCache_Dec *restrict bitcache, crc32_dec *restrict crc,
	rice24_dec
)
/*@modifies	*value,
		*rice,
		*bitcache,
		*crc
@*/
;

#undef unary
#undef cache
#undef count
#undef crc
ALWAYS_INLINE size_t rice24_read_unary(
	/*@out@*/ rice24_dec *restrict unary, const u8 *restrict, size_t,
	cache32 *restrict cache, bitcnt_dec *restrict count,
	crc32_dec *restrict crc, rice24_dec
)
/*@modifies	*unary,
		*cache,
		*count,
		*crc
@*/
;

#undef binary
#undef cache
#undef count
#undef crc
ALWAYS_INLINE size_t rice24_read_binary(
	/*@out@*/ rice24_dec *restrict binary, const u8 *restrict, size_t,
	cache32 *restrict cache, bitcnt_dec *restrict count,
	crc32_dec *restrict crc, bitcnt_dec
)
/*@modifies	*binary,
		*cache,
		*count,
		*crc
@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn get_unary_lax_limit
 * @brief max number (plus 8u) of 1-bits in a unary code
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return max number (plus 8u) of 1-bits in a unary code
**/
ALWAYS_INLINE CONST rice24_dec
get_unary_lax_limit(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
	case LIBTTAr_SAMPLEBYTES_2:
		return UNARY_LAX_LIMIT_1_2;
	case LIBTTAr_SAMPLEBYTES_3:
		return UNARY_LAX_LIMIT_3;
	}
	UNREACHABLE;
}

#ifndef NDEBUG
/**@fn get_rice24_enc_max
 * @brief max number of bytes rice24_encode could write
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return max number of bytes rice24_encode could write
**/
ALWAYS_INLINE CONST size_t
get_rice24_enc_max(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
	case LIBTTAr_SAMPLEBYTES_2:
		return RICE24_ENC_MAX_1_2;
	case LIBTTAr_SAMPLEBYTES_3:
		return RICE24_ENC_MAX_3;
	}
	UNREACHABLE;
}

/**@fn get_rice24_dec_max
 * @brief max number of bytes rice24_decode could read
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return max number of bytes rice24_decode could read
**/
ALWAYS_INLINE CONST size_t
get_rice24_dec_max(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
	case LIBTTAr_SAMPLEBYTES_2:
		return RICE24_DEC_MAX_1_2;
	case LIBTTAr_SAMPLEBYTES_3:
		return RICE24_DEC_MAX_3;
	}
	UNREACHABLE;
}
#endif

//==========================================================================//

/**@fn binexp32
 * @brief binary exponentiation 32-bit (2**'k')
 *
 * @param k bit number
 *
 * @return a 32-bit mask with only the 'k'th bit set
**/
ALWAYS_INLINE CONST u32f
binexp32(const bitcnt k)
/*@*/
{
	return (((u32f) 0x1u) << k);
}

/**@fn lsmask32
 * @brief least significant mask 32-bit
 *   macro'd because of fast types (almost like in TBCNT8)
 *
 * @param k number of bits in the mask
 *
 * @return a mask with 'k' low bits set
 *
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
#ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
ALWAYS_INLINE CONST u32f
lsmask32(const bitcnt k)
/*@*/
{
	return (u32f) (((0x1u) << k) - 1u);
}
#else
ALWAYS_INLINE CONST u32
lsmask32(const bitcnt k)
/*@*/
{
	return lsmask32_table[k];
}
#endif	// LIBTTAr_OPT_PREFER_LOOKUP_TABLES

/**@fn BZHI32
 * @brief (bit-manip) zero high bits by index 32-bit
 *   macro'd because of fast types (almost like in TBCNT8)
 *
 * @param x input value
 * @param k bit index
 *
 * @return 'x' zero'd from the 'k'th bit to the most significant bit
 *
 * @note both clang and gcc understand this.
**/
#define BZHI32(x, k)	((x) & lsmask32((bitcnt) (k)))

/**@fn TBCNT8
 * @brief trailing bit count 8-bit
 *   macro'd because fast types were causing extra casts with inline functions
 *
 * @param x value to count
 *
 * @return number of trailing bits
 *
 * @note 'x' will never be UINT[32|64]_MAX
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
#ifndef TBCNT8_TABLE
#if HAS_BUILTIN(BUILTIN_TZCNT32)
#define TBCNT8(x)	BUILTIN_TZCNT32(~((u32) (x)))
#elif HAS_BUILTIN(BUILTIN_TZCNT64)
#define TBCNT8(x)	BUILTIN_TZCNT64(~((u64) (x)))
#else
#error "TBCNT8"
#endif
#else // defined(TBCNT8_TABLE)
#define TBCNT8(x)	tbcnt8_table[(x)]
#endif // TBCNT8_TABLE

//==========================================================================//

/**@fn rice24_crc32_enc
 * @brief add a byte to a CRC; encode version
 *
 * @param x input byte
 * @param crc[in out] current CRC
 *
 * @return 'x'
**/
ALWAYS_INLINE u8
rice24_crc32_enc(const u8 x, crc32_enc *const restrict crc)
/*@modifies	*crc@*/
{
	*crc = crc32_cont_enc(x, *crc);
	return x;
}

/**@fn rice24_crc32_dec
 * @brief add a byte to a CRC; decode version
 *
 * @see rice24_crc32_enc()
**/
ALWAYS_INLINE u8
rice24_crc32_dec(const u8 x, crc32_dec *const restrict crc)
/*@modifies	*crc@*/
{
	*crc = crc32_cont_dec(x, *crc);
	return x;
}

//==========================================================================//

#define RICE24_UPDATE_BODY(Xtype) { \
	assert(*k <= (Xtype) 24u); \
	\
	*sum += value - (*sum >> 4u);	/* may wrap */ \
	if IMPROBABLE ( *sum < test[0u], 0.027 ){ \
		*k -= 1u; \
	} \
	else if IMPROBABLE ( *sum > test[1u], 0.027 ){ \
		*k += 1u; \
	} else{;} \
	\
	assert(*k <= (Xtype) 24u); \
	return; \
}

/**@fn rice24_update_enc
 * @brief update the rice state; encode version
 *
 * @param sum[in out] rice->sum[]
 * @param k[in out] rice->k[]
 * @param value input value to code
 * @param test[in] binexp32p4 comparison values
 *
 * @pre  *k <= (bitcnt_enc) 24u
 * @post *k <= (bitcnt_enc) 24u
**/
ALWAYS_INLINE void
rice24_update_enc(
	u32 *const restrict sum, bitcnt_enc *const restrict k,
	const u32 value, const u32 *const restrict test
)
/*@modifies	*sum,
		*k
@*/
{
	RICE24_UPDATE_BODY(bitcnt_enc);
}

/**@fn rice24_update_dec
 * @brief update the rice state; decode version
 *
 * @see rice24_update_enc()
**/
ALWAYS_INLINE void
rice24_update_dec(
	u32 *const restrict sum, bitcnt_dec *const restrict k,
	const u32 value, const u32 *const restrict test
)
/*@modifies	*sum,
		*k
@*/
{
	RICE24_UPDATE_BODY(bitcnt_dec);
}

//==========================================================================//

/**@fn rice24_encode
 * @brief encode a filtered i32 value into rice codes
 *
 * @param dest[out] destination buffer
 * @param value value to encode
 * @param nbytes_enc total number of bytes encoded so far; index of 'dest'
 * @param rice[in out] the rice code data for the current channel
 * @param bitcache[in out] the bitcache data
 * @param crc[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size (unary + binary):
 *     8/16-bit :    8199u
 *       24-bit : 2097159uL
**/
ALWAYS_INLINE size_t
rice24_encode(
	/*@reldef@*/ u8 *const restrict dest, u32 value,
	size_t nbytes_enc, struct Rice_Enc *const restrict rice,
	struct BitCache_Enc *const restrict bitcache,
	crc32_enc *const restrict crc
)
/*@modifies	*dest,
		*rice,
		*bitcache,
		*crc
@*/
{
	u32        *const restrict sum0  = &rice->sum[0u];
	u32        *const restrict sum1  = &rice->sum[1u];
	bitcnt_enc *const restrict k0    = &rice->k[0u];
	bitcnt_enc *const restrict k1    = &rice->k[1u];
	cache64    *const restrict cache = &bitcache->cache;
	bitcnt_enc *const restrict count = &bitcache->count;

	rice24_enc unary, binary;
	bitcnt_enc bin_k;
	const u32 *test0, *test1;

	#define RICE24_ENCODE_STATE_0(Xvalue) { \
		bin_k      = *k0; \
		test0      = &binexp32p4_table[*k0]; \
		rice24_update_enc(sum0, k0, (Xvalue), test0); \
	}
	#define RICE24_ENCODE_STATE_1(Xvalue) { \
		(Xvalue)  -= binexp32((bitcnt) bin_k); \
		bin_k      = *k1; \
		test1      = &binexp32p4_table[*k1]; \
		rice24_update_enc(sum1, k1, (Xvalue), test1); \
	}
	#define RICE24_ENCODE_UNARY(Xunary) { \
		unary      = (Xunary); \
		nbytes_enc = rice24_write_unary( \
			dest, unary, nbytes_enc, cache, count, crc \
		); \
	}
	#define RICE24_ENCODE_UNARY_ZERO { \
		nbytes_enc = rice24_write_unary_zero( \
			dest, nbytes_enc, cache, count, crc \
		); \
	}
	#define RICE24_ENCODE_BINARY(Xbinary) { \
		binary     = (Xbinary); \
		rice24_cache_binary(binary, bin_k, cache, count); \
	}

	// value + state
	RICE24_ENCODE_STATE_0(value);
	if PROBABLE ( value >= binexp32((bitcnt) bin_k), 0.575 ){
		RICE24_ENCODE_STATE_1(value);

		// unary + binary
		RICE24_ENCODE_UNARY((rice24_enc) ((value >> bin_k) + 1u));
		RICE24_ENCODE_BINARY((rice24_enc) BZHI32(value, bin_k));
	}
	else {	// unary-zero + binary
		RICE24_ENCODE_UNARY_ZERO;
		RICE24_ENCODE_BINARY((rice24_enc) BZHI32(value, bin_k));
	}

	return nbytes_enc;
}

/**@fn rice24_encode_cacheflush
 * @brief flush any data left in the 'bitcache' to 'dest'
 *
 * @param dest[out] destination buffer
 * @param nbytes_enc total number of bytes encoded so far; index of 'dest'
 * @param bitcache[in out] the bitcache data
 * @param crc_inout[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @pre bitcache->count <= (bitcnt_enc) 63u
 *
 * @note max write size: 8u
**/
ALWAYS_INLINE size_t
rice24_encode_cacheflush(
	/*@reldef@*/ u8 *const restrict dest, size_t nbytes_enc,
	struct BitCache_Enc *const restrict bitcache,
	u32 *const restrict crc_inout
)
/*@modifies	*dest,
		*bitcache,
		*crc_inout
@*/
{
	cache64    *const restrict cache = &bitcache->cache;
	bitcnt_enc *const restrict count = &bitcache->count;
	crc32_enc crc = (crc32_enc) *crc_inout;

	assert(*count <= (bitcnt_enc) 63u);

	nbytes_enc = rice24_write_cache(
		dest, nbytes_enc, cache, count, &crc, (bitcnt_enc) 7u
	);
	*count = 0u;

	*crc_inout = (u32) crc;
	return nbytes_enc;
}

//--------------------------------------------------------------------------//

/**@fn rice24_write_unary
 * @brief write the 'cache' to 'dest' and put the unary code in the 'cache'
 *
 * @param dest[out] destination buffer
 * @param nbytes_enc total number of bytes encoded so far; index of 'dest'
 * @param unary the unary code
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @pre  *count <= (bitcnt_enc) 63u
 * @post *count <= (bitcnt_enc) 39u
 *
 * @note max write size (unary + cache):
 *	 8/16-bit :    8192u + 7u ==    8199u
 *	   24-bit : 2097152u + 7u == 2097159uL
**/
ALWAYS_INLINE size_t
rice24_write_unary(
	/*@reldef@*/ u8 *const restrict dest, rice24_enc unary,
	size_t nbytes_enc, cache64 *const restrict cache,
	bitcnt_enc *const restrict count, crc32_enc *const restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	assert(*count <= (bitcnt_enc) 63u);

	goto loop_entr;
	do {	unary  -= 32u;
		*cache |= ((cache64) UINT32_MAX) << *count;
		*count += 32u;
loop_entr:
		nbytes_enc = rice24_write_cache(
			dest, nbytes_enc, cache, count, crc, 0
		);
	} while UNLIKELY ( unary >= (rice24_enc) 32u );

	*cache |= ((cache64) lsmask32((bitcnt) unary)) << *count;
	*count += (bitcnt_enc) (unary + 1u);	// + terminator

	assert(*count <= (bitcnt_enc) 39u);
	return nbytes_enc;
}

/**@fn rice24_write_unary_zero
 * @brief write the 'cache' to 'dest' and put a zero unary code in the 'cache'
 *
 * @param dest[out] destination buffer
 * @param nbytes_enc total number of bytes encoded so far; index of 'dest'
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @pre  *count <= (bitcnt_enc) 63u
 * @post *count <= (bitcnt_enc)  8u
 *
 * @note max write size: 7u
**/
ALWAYS_INLINE size_t
rice24_write_unary_zero(
	/*@reldef@*/ u8 *const restrict dest, size_t nbytes_enc,
	cache64 *const restrict cache, bitcnt_enc *const restrict count,
	crc32_enc *const restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	assert(*count <= (bitcnt_enc) 63u);

	nbytes_enc = rice24_write_cache(
		dest, nbytes_enc, cache, count, crc, 0
	);

	*count += 1u;	// + terminator

	assert(*count <= (bitcnt_enc)  8u);
	return nbytes_enc;
}

/**@fn rice24_cache_binary
 * @brief write a binary code to the 'cache'
 *
 * @param binary the binary code
 * @param bin_k number of bits in the binary code
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 *
 * @pre  binary <= (bitcnt_enc) UINT24_MAX
 * @pre   bin_k <= (bitcnt_enc) 24u
 * @pre  *count <= (bitcnt_enc) 39u	// 39u unary, 8u unary-zero
 * @post *count <= (bitcnt_enc) 63u
 *
 * @note max bitcache write size: 3u
**/
ALWAYS_INLINE void
rice24_cache_binary(
	const rice24_enc binary, const bitcnt_enc bin_k,
	cache64 *const restrict cache, bitcnt_enc *const restrict count
)
/*@modifies	*cache,
		*count
@*/
{
	assert(binary <= (rice24_enc) UINT24_MAX);
	assert( bin_k <= (bitcnt_enc) 24u);
	assert(*count <= (bitcnt_enc) 39u);

	*cache |= ((cache64) binary) << *count;
	*count += bin_k;

	assert(*count <= (bitcnt_enc) 63u);
	return;
}

/**@fn rice24_write_cache
 * @brief write ('*count' / 8u) bytes to 'dest' from the bitcache
 *
 * @param dest[out] destination buffer
 * @param nbytes_enc total number of bytes encoded so far; index of 'dest'
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 *
 * @pre  *count <= (bitcnt_enc) 70u
 * @pre  (extra == (bitcnt_enc)  0u) || (extra == (bitcnt_enc)  7u)
 * @post *count <= (bitcnt_enc)  7u
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size: 8u
**/
ALWAYS_INLINE size_t
rice24_write_cache(
	/*@reldef@*/ u8 *const restrict dest, size_t nbytes_enc,
	cache64 *const restrict cache, bitcnt_enc *const restrict count,
	crc32_enc *const restrict crc, const bitcnt_enc extra
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	assert(*count <= (bitcnt_enc) 70u);
	assert((extra == (bitcnt_enc)  0u) || (extra == (bitcnt_enc)  7u));

	*count += extra;
	while PROBABLE ( *count >= (bitcnt_enc) 8u, 0.9 ){
		dest[nbytes_enc++] = rice24_crc32_enc((u8) *cache, crc);
		*cache >>= 8u;
		*count  -= 8u;
	}

	assert(*count <= (bitcnt_enc)  7u);
	return nbytes_enc;
}

//==========================================================================//

/**@fn rice24_decode
 * @brief decode rice codes into a filtered i32 value
 *
 * @param value[in out] value to decode
 * @param src[in] source buffer
 * @param nbytes_dec total number of bytes decoded so far; index of 'src'
 * @param rice[in out] the rice code data for the current channel
 * @param bitcache[in out] the bitcache data
 * @param crc[in out] the current CRC
 * @param unary_lax_limit limit for the unary code
 *
 * @return number of bytes read from 'src' + 'nbytes_dec'
 *
 * @note max read size (unary + binary):
 *     8/16-bit :    8197u
 *       24-bit : 2097157uL
**/
ALWAYS_INLINE size_t
rice24_decode(
	/*@out@*/ u32 *const restrict value, const u8 *const restrict src,
	size_t nbytes_dec, struct Rice_Dec *const restrict rice,
	struct BitCache_Dec *const restrict bitcache,
	crc32_dec *const restrict crc, const rice24_dec unary_lax_limit
)
/*@modifies	*value,
		*rice,
		*bitcache,
		*crc
@*/
{
	u32        *const restrict sum0  = &rice->sum[0u];
	u32        *const restrict sum1  = &rice->sum[1u];
	bitcnt_dec *const restrict k0    = &rice->k[0u];
	bitcnt_dec *const restrict k1    = &rice->k[1u];
	cache32    *const restrict cache = &bitcache->cache;
	bitcnt_dec *const restrict count = &bitcache->count;

	rice24_dec unary, binary;
	bitcnt_dec bin_k;
	const u32 *test0, *test1;

	#define RICE24_DECODE_UNARY(Xunary) { \
		nbytes_dec = rice24_read_unary( \
			(Xunary), src, nbytes_dec, cache, count, crc, \
			unary_lax_limit \
		); \
	}
	#define RICE24_DECODE_BINARY(Xbinary) { \
		nbytes_dec = rice24_read_binary( \
			(Xbinary), src, nbytes_dec, cache, count, crc, bin_k \
		); \
	}
	#define RICE24_DECODE_STATE_1(Xvalue) { \
		rice24_update_dec(sum1, k1, (Xvalue), test1); \
		(Xvalue)  += binexp32((bitcnt) *k0); \
	}
	#define RICE24_DECODE_STATE_0(Xvalue) { \
		rice24_update_dec(sum0, k0, (Xvalue), test0); \
	}

	// unary
	RICE24_DECODE_UNARY(&unary);
	if PROBABLE ( unary != 0, 0.575 ){
		bin_k   = *k1;
		test1   = &binexp32p4_table[*k1];
		test0   = &binexp32p4_table[*k0];

		// binary
		RICE24_DECODE_BINARY(&binary);

		// value + state
		*value  = (u32) (((unary - 1u) << bin_k) + binary);
		RICE24_DECODE_STATE_1(*value);
		RICE24_DECODE_STATE_0(*value);
	}
	else {	bin_k   = *k0;
		test0   = &binexp32p4_table[*k0];

		// binary
		RICE24_DECODE_BINARY(&binary);

		// value + state
		*value  = (u32) binary;
		RICE24_DECODE_STATE_0(*value);
	}

	return nbytes_dec;
}

//--------------------------------------------------------------------------//

/**@fn rice24_read_unary
 * @brief read a unary code from 'src'
 *
 * @param unary[out] the unary code
 * @param src[in] source buffer
 * @param nbytes_dec total number of bytes decoded so far; index of 'src'
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 * @param lax_limit limit for the unary code. has an extra byte of margin, so
 *   if it is surpased, then the data is definitely invalid (corrupt or
 *   malicious). this would be caused by an overly long string of 0xFFu bytes
 *   in the source
 *
 * @return number of bytes read from 'src' + 'nbytes_dec'
 *
 * @pre  *count <= (bitcnt_dec) 7u
 * @post *count <= (bitcnt_dec) 7u
 *
 * @note max read size:
 *     8/16-bit :    8194u
 *       24-bit : 2097154uL
**/
ALWAYS_INLINE size_t
rice24_read_unary(
	/*@out@*/ rice24_dec *const restrict unary,
	const u8 *const restrict src, size_t nbytes_dec,
	cache32 *const restrict cache, bitcnt_dec *const restrict count,
	crc32_dec *const restrict crc, const rice24_dec lax_limit
)
/*@modifies	*unary,
		*cache,
		*count,
		*crc
@*/
{
	bitcnt_dec nbits;
	u8 inbyte;

	assert(*count <= (bitcnt_dec) 7u);

	nbits  = (bitcnt_dec) TBCNT8(*cache);
	*unary = (rice24_dec) nbits;
	if IMPROBABLE ( nbits == *count, 0.25 ){
		do {	inbyte  = rice24_crc32_dec(src[nbytes_dec++], crc);
			nbits   = (bitcnt_dec) TBCNT8(inbyte);
			*unary += nbits;
			if UNLIKELY ( *unary > lax_limit ){
				nbits = 0;	// prevents *count underflow
				break;
			}
		} while UNLIKELY ( nbits == (bitcnt_dec) 8u );
		*cache = (cache32) inbyte;
		*count = (bitcnt_dec) 8u;
	}
	*cache >>= nbits + 1u;	// + terminator
	*count  -= nbits + 1u;	// ~

	assert(*count <= (bitcnt_dec) 7u);
	return nbytes_dec;
}

/**@fn rice24_read_binary
 * @brief read a binary code from 'src'
 *
 * @param binary[out] the binary code
 * @param src[in] source buffer
 * @param nbytes_dec total number of bytes decoded so far; index of 'src'
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 * @param bin_k number of bits in the binary code
 *
 * @return number of bytes read from 'src' + 'nbytes_dec'
 *
 * @pre  *count  <= (bitcnt_dec)  7u
 * @pre   bin_k  <= (bitcnt_dec) 24u
 * @post *binary <= (rice24_enc) UINT24_MAX
 * @post *count  <= (bitcnt_dec)  7u
 *
 * @note max read size: 3u
 *   a check like in the unary reader is unnecessary
**/
ALWAYS_INLINE size_t
rice24_read_binary(
	/*@out@*/ rice24_dec *const restrict binary,
	const u8 *const restrict src, size_t nbytes_dec,
	cache32 *const restrict cache, bitcnt_dec *const restrict count,
	crc32_dec *const restrict crc, const bitcnt_dec bin_k
)
/*@modifies	*binary,
		*cache,
		*count,
		*crc
@*/
{
	u8 inbyte;

	assert( bin_k  <= (bitcnt_dec) 24u);
	assert(*count  <= (bitcnt_dec)  7u);

	while PROBABLE ( *count < bin_k, 0.9 ){
		inbyte  = rice24_crc32_dec(src[nbytes_dec++], crc);
		*cache |= ((cache32) inbyte) << *count;
		*count += 8u;
	}
	*binary  = (rice24_dec) BZHI32(*cache, bin_k);
	*cache >>= bin_k;
	*count  -= bin_k;

	assert(*binary <= (rice24_enc) UINT24_MAX);
	assert(*count  <= (bitcnt_dec) 7u);
	return nbytes_dec;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
