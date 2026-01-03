#ifndef H_TTA_CODEC_RICE24_H
#define H_TTA_CODEC_RICE24_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/rice24.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2026, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "./common.h"
#include "./crc32.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

#define UINT24_MAX		UINT32_C(0xFFFFFF)

/* lax_limit has an extra byte to make handling invalid data faster/easier */
#define UNARY_LAX_LIMIT_1_2	((rice24_dec) ((8u * UINT32_C(   8194)) - 1u))
#define UNARY_LAX_LIMIT_3	((rice24_dec) ((8u * UINT32_C(2097154)) - 1u))

/* max unary + binary r/w size for one value */
#define RICE24_ENC_MAX_1_2	SIZE_C(   8199)
#define RICE24_ENC_MAX_3	SIZE_C(2097159)
#define RICE24_DEC_MAX_1_2	SIZE_C(   8197)
#define RICE24_DEC_MAX_3	SIZE_C(2097157)

/* //////////////////////////////////////////////////////////////////////// */

/*@-redef@*/

/*@unchecked@*/
BUILD_HIDDEN
BUILD_EXTERN const uint32_t binexp32p4_table[26u];

#ifdef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
/*@unchecked@*/
BUILD_HIDDEN
BUILD_EXTERN const uint32_t lsmask32_table[32u];
#endif	/* LIBTTAr_OPT_PREFER_LOOKUP_TABLES */

#ifdef USE_TBCNT8_TABLE
/*@unchecked@*/
BUILD_HIDDEN
BUILD_EXTERN const bitcnt_dec tbcnt8_table[256u];
#endif	/* USE_TBCNT8_TABLE */

/*@=redef@*/

/* //////////////////////////////////////////////////////////////////////// */

enum WriteCacheMode {
	WRITECACHE_BYTES	= 0u,
	WRITECACHE_FLUSH	= 7u
};

/* //////////////////////////////////////////////////////////////////////// */

CONST
ALWAYS_INLINE rice24_dec get_unary_lax_limit(enum LibTTAr_SampleBytes) /*@*/;

#ifndef NDEBUG
CONST
ALWAYS_INLINE size_t get_rice24_enc_max(enum LibTTAr_SampleBytes) /*@*/;

CONST
ALWAYS_INLINE size_t get_rice24_dec_max(enum LibTTAr_SampleBytes) /*@*/;
#endif	/* NDEBUG */

/* ------------------------------------------------------------------------ */

CONST
ALWAYS_INLINE uint_fast32_t binexp32(bitcnt) /*@*/;

#ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
CONST
ALWAYS_INLINE uint_fast32_t lsmask32(bitcnt) /*@*/;
#else
CONST
ALWAYS_INLINE uint32_t lsmask32(bitcnt) /*@*/;
#endif	/* LIBTTAr_OPT_PREFER_LOOKUP_TABLES */

/* ------------------------------------------------------------------------ */

#undef crc
ALWAYS_INLINE uint8_t rice24_crc32_enc(uint8_t, crc32_enc *RESTRICT crc)
/*@modifies	*crc@*/
;

#undef crc
ALWAYS_INLINE uint8_t rice24_crc32_dec(uint8_t, crc32_dec *RESTRICT crc)
/*@modifies	*crc@*/
;

/* ------------------------------------------------------------------------ */

#undef sum
#undef k
ALWAYS_INLINE void rice24_update_enc(
	uint32_t *RESTRICT sum, bitcnt_enc *RESTRICT k, uint32_t,
	const uint32_t *RESTRICT
)
/*@modifies	*sum,
		*k
@*/
;

#undef sum
#undef k
ALWAYS_INLINE void rice24_update_dec(
	uint32_t *RESTRICT sum, bitcnt_dec *RESTRICT k, uint32_t,
	const uint32_t *RESTRICT
)
/*@modifies	*sum,
		*k
@*/
;

/* ------------------------------------------------------------------------ */

#undef dest
#undef rice
#undef bitcache
#undef crc
ALWAYS_INLINE size_t rice24_encode(
	/*@reldef@*/ uint8_t *RESTRICT dest, uint32_t, size_t,
	struct Rice_Enc *RESTRICT rice,
	struct BitCache_Enc *RESTRICT bitcache, crc32_enc *RESTRICT crc
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
	/*@reldef@*/ uint8_t *RESTRICT dest, size_t,
	struct BitCache_Enc *RESTRICT bitcache, uint32_t *RESTRICT crc_inout
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
	/*@reldef@*/ uint8_t *RESTRICT dest, rice24_enc, size_t,
	cache64 *RESTRICT cache, bitcnt_enc *RESTRICT count,
	crc32_enc *RESTRICT crc
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
	/*@reldef@*/ uint8_t *RESTRICT dest, size_t, cache64 *RESTRICT cache,
	bitcnt_enc *RESTRICT count, crc32_enc *RESTRICT crc
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
	rice24_enc, bitcnt_enc, cache64 *RESTRICT cache,
	bitcnt_enc *RESTRICT count
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
	/*@reldef@*/ uint8_t *RESTRICT dest, size_t, cache64 *RESTRICT cache,
	bitcnt_enc *RESTRICT count, crc32_enc *RESTRICT crc,
	enum WriteCacheMode
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
;

/* ------------------------------------------------------------------------ */

#undef value
#undef rice
#undef bitcache
#undef crc
ALWAYS_INLINE size_t rice24_decode(
	/*@out@*/ uint32_t *RESTRICT value, const uint8_t *RESTRICT, size_t,
	struct Rice_Dec *RESTRICT rice,
	struct BitCache_Dec *RESTRICT bitcache, crc32_dec *RESTRICT crc,
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
	/*@out@*/ rice24_dec *RESTRICT unary, const uint8_t *RESTRICT, size_t,
	cache32 *RESTRICT cache, bitcnt_dec *RESTRICT count,
	crc32_dec *RESTRICT crc, rice24_dec
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
	/*@out@*/ rice24_dec *RESTRICT binary, const uint8_t *RESTRICT,
	size_t, cache32 *RESTRICT cache, bitcnt_dec *RESTRICT count,
	crc32_dec *RESTRICT crc, bitcnt_dec
)
/*@modifies	*binary,
		*cache,
		*count,
		*crc
@*/
;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn get_unary_lax_limit
 * @brief max number (plus 8u) of 1-bits in a unary code
 *
 * @param samplebytes - number of bytes per PCM sample
 *
 * @return max number (plus 8u) of 1-bits in a unary code
**/
CONST
ALWAYS_INLINE rice24_dec
get_unary_lax_limit(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	SAMPLEBYTES_RANGE_ASSERT(samplebytes);

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
 * @param samplebytes - number of bytes per PCM sample
 *
 * @return max number of bytes rice24_encode could write
**/
CONST
ALWAYS_INLINE size_t
get_rice24_enc_max(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	SAMPLEBYTES_RANGE_ASSERT(samplebytes);

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
 * @param samplebytes - number of bytes per PCM sample
 *
 * @return max number of bytes rice24_decode could read
**/
CONST
ALWAYS_INLINE size_t
get_rice24_dec_max(const enum LibTTAr_SampleBytes samplebytes)
/*@*/
{
	SAMPLEBYTES_RANGE_ASSERT(samplebytes);

	switch ( samplebytes ){
	case LIBTTAr_SAMPLEBYTES_1:
	case LIBTTAr_SAMPLEBYTES_2:
		return RICE24_DEC_MAX_1_2;
	case LIBTTAr_SAMPLEBYTES_3:
		return RICE24_DEC_MAX_3;
	}
	UNREACHABLE;
}

#endif	/* NDEBUG */

/* ======================================================================== */

/**@fn binexp32
 * @brief binary exponentiation 32-bit (2**'k')
 *
 * @param k - bit number
 *
 * @return a 32-bit mask with only the 'k'th bit set
**/
CONST
ALWAYS_INLINE uint_fast32_t
binexp32(const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 31u);

	return (uint_fast32_t) (((uint_fast32_t) 0x1u) << k);
}

/**@fn lsmask32
 * @brief least significant mask 32-bit
 *
 * @param k - number of bits in the mask
 *
 * @return a mask with 'k' low bits set
 *
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
#ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
CONST
ALWAYS_INLINE uint_fast32_t
lsmask32(const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 31u);

	return (uint_fast32_t) ((((uint_fast32_t) 0x1u) << k) - 1u);
}
#else	/* defined(LIBTTAr_OPT_PREFER_LOOKUP_TABLES) */
CONST
ALWAYS_INLINE uint32_t
lsmask32(const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 31u);

	return lsmask32_table[k];
}
#endif	/* LIBTTAr_OPT_PREFER_LOOKUP_TABLES */

/**@fn BZHI32
 * @brief (bit-manip) zero high bits by index 32-bit
 *   macro'd because of fast types (almost like in TBCNT8)
 *
 * @param x - input value
 * @param k - bit index
 *
 * @return 'x' zero'd from the 'k'th bit to the most significant bit
 *
 * @note both clang and gcc understand this.
**/
#ifdef NDEBUG
#define BZHI32(x_x, x_k)	((x_x) & lsmask32((bitcnt) (x_k)))
#else
CONST
ALWAYS_INLINE uint_fast32_t
X_BZHI32(const uint_fast32_t x, const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 31u);

	return x & lsmask32(k);
}
#define BZHI32(x_x, x_k)	X_BZHI32((uint_fast32_t) (x_x), (x_k))
#endif	/* BZHI32 */

/**@fn TBCNT8
 * @brief trailing bit count 8-bit
 *   macro'd because fast types were causing extra casts with inline functions
 *
 * @param x - value to count
 *
 * @return number of trailing bits
**/
#ifndef USE_TBCNT8_TABLE
#if defined(HAS_BUILTIN_CTZ32)
#define X_TBCNT8(x_x)		BUILTIN_CTZ32(~((uint32_t) (x_x)))
#elif defined(HAS_BUILTIN_CTZ64)
#define X_TBCNT8(x_x)		BUILTIN_CTZ64(~((uint64_t) (x_x)))
#else
#error "TBCNT8"
#endif	/* BUILTIN_CTZ32 || BUILTIN_CTZ64 */
#else	/* defined(USE_TBCNT8_TABLE) */
#define X_TBCNT8(x_x)		tbcnt8_table[(x_x)]
#endif	/* USE_TBCNT8_TABLE */

#ifdef NDEBUG
#define TBCNT8(x_x)		X_TBCNT8(x_x)
#else
CONST
ALWAYS_INLINE bitcnt
TBCNT8(const uint8_t x)
/*@*/
{
	return (bitcnt) X_TBCNT8(x);
}
#endif	/* TBCNT8 */

/* ======================================================================== */

/**@fn rice24_crc32_enc
 * @brief add a byte to a CRC; encode version
 *
 * @param x   - input byte
 * @param crc - current CRC
 *
 * @return 'x'
**/
ALWAYS_INLINE uint8_t
rice24_crc32_enc(const uint8_t x, crc32_enc *const RESTRICT crc)
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
ALWAYS_INLINE uint8_t
rice24_crc32_dec(const uint8_t x, crc32_dec *const RESTRICT crc)
/*@modifies	*crc@*/
{
	*crc = crc32_cont_dec(x, *crc);

	return x;
}

/* ======================================================================== */

#define RICE24_UPDATE_BODY(x_type) { \
	assert(*k <= (x_type) 24u); \
	\
	*sum += value - (*sum >> 4u);	/* may wrap */ \
	if IMPROBABLE ( *sum < test[0u], 0.027 ){ \
		*k -= 1u; \
	} \
	else if IMPROBABLE ( *sum > test[1u], 0.027 ){ \
		*k += 1u; \
	} else{;} \
	\
	assert(*k <= (x_type) 24u); \
	return; \
}

/**@fn rice24_update_enc
 * @brief update the rice state; encode version
 *
 * @param sum   - rice->sum[]
 * @param k     - rice->k[]
 * @param value - input value to code
 * @param test  - binexp32p4 comparison values
**/
ALWAYS_INLINE void
rice24_update_enc(
	uint32_t *const RESTRICT sum, bitcnt_enc *const RESTRICT k,
	const uint32_t value, const uint32_t *const RESTRICT test
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
	uint32_t *const RESTRICT sum, bitcnt_dec *const RESTRICT k,
	const uint32_t value, const uint32_t *const RESTRICT test
)
/*@modifies	*sum,
		*k
@*/
{
	RICE24_UPDATE_BODY(bitcnt_dec);
}

/* ======================================================================== */

/**@fn rice24_encode
 * @brief encode a filtered i32 value into rice codes
 *
 * @param dest       - destination buffer
 * @param value      - value to encode
 * @param nbytes_enc - total number of bytes encoded so far; index of 'dest'
 * @param rice       - rice code data for the current channel
 * @param bitcache   - bitcache data
 * @param crc        - current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size (unary + binary):
 *     8/16-bit :    8199u
 *       24-bit : 2097159uL
**/
ALWAYS_INLINE size_t
rice24_encode(
	/*@reldef@*/ uint8_t *const RESTRICT dest, uint32_t value,
	size_t nbytes_enc, struct Rice_Enc *const RESTRICT rice,
	struct BitCache_Enc *const RESTRICT bitcache,
	crc32_enc *const RESTRICT crc
)
/*@modifies	*dest,
		*rice,
		*bitcache,
		*crc
@*/
{
	uint32_t   *const RESTRICT sum0  = &rice->sum[0u];
	uint32_t   *const RESTRICT sum1  = &rice->sum[1u];
	bitcnt_enc *const RESTRICT k0    = &rice->k[0u];
	bitcnt_enc *const RESTRICT k1    = &rice->k[1u];
	cache64    *const RESTRICT cache = &bitcache->cache;
	bitcnt_enc *const RESTRICT count = &bitcache->count;
	/* * */
	rice24_enc unary, binary;
	bitcnt_enc bin_k;
	const uint32_t *test0, *test1;

	#define RICE24_ENCODE_STATE_0(x_value) { \
		bin_k      = *k0; \
		test0      = &binexp32p4_table[*k0]; \
		rice24_update_enc(sum0, k0, (x_value), test0); \
	}
	#define RICE24_ENCODE_STATE_1(x_value) { \
		(x_value) -= binexp32((bitcnt) bin_k); \
		bin_k      = *k1; \
		test1      = &binexp32p4_table[*k1]; \
		rice24_update_enc(sum1, k1, (x_value), test1); \
	}
	#define RICE24_ENCODE_UNARY(x_unary) { \
		unary      = (x_unary); \
		nbytes_enc = rice24_write_unary( \
			dest, unary, nbytes_enc, cache, count, crc \
		); \
	}
	#define RICE24_ENCODE_UNARY_ZERO { \
		nbytes_enc = rice24_write_unary_zero( \
			dest, nbytes_enc, cache, count, crc \
		); \
	}
	#define RICE24_ENCODE_BINARY(x_binary) { \
		binary     = (x_binary); \
		rice24_cache_binary(binary, bin_k, cache, count); \
	}

	/* value + state */
	RICE24_ENCODE_STATE_0(value);
	if PROBABLE ( value >= binexp32((bitcnt) bin_k), 0.575 ){
		RICE24_ENCODE_STATE_1(value);

		/* unary + binary */
		RICE24_ENCODE_UNARY((rice24_enc) ((value >> bin_k) + 1u));
		RICE24_ENCODE_BINARY((rice24_enc) BZHI32(value, bin_k));
	}
	else {	/* unary-zero + binary */
		RICE24_ENCODE_UNARY_ZERO;
		RICE24_ENCODE_BINARY((rice24_enc) BZHI32(value, bin_k));
	}
	return nbytes_enc;
}

/**@fn rice24_encode_cacheflush
 * @brief flush any data left in the 'bitcache' to 'dest'
 *
 * @param dest       - destination buffer
 * @param nbytes_enc - total number of bytes encoded so far; index of 'dest'
 * @param bitcache   - bitcache data
 * @param crc_inout  - current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size: 8u
**/
ALWAYS_INLINE size_t
rice24_encode_cacheflush(
	/*@reldef@*/ uint8_t *const RESTRICT dest, size_t nbytes_enc,
	struct BitCache_Enc *const RESTRICT bitcache,
	uint32_t *const RESTRICT crc_inout
)
/*@modifies	*dest,
		*bitcache,
		*crc_inout
@*/
{
	cache64    *const RESTRICT cache = &bitcache->cache;
	bitcnt_enc *const RESTRICT count = &bitcache->count;
	/* * */
	crc32_enc crc = (crc32_enc) *crc_inout;

	assert(*count <= (bitcnt_enc) 63u);

	nbytes_enc = rice24_write_cache(
		dest, nbytes_enc, cache, count, &crc, WRITECACHE_FLUSH
	);
	*count = 0;

	*crc_inout = (uint32_t) crc;
	return nbytes_enc;
}

/* ------------------------------------------------------------------------ */

/**@fn rice24_write_unary
 * @brief write the 'cache' to 'dest' and put the unary code in the 'cache'
 *
 * @param dest       - destination buffer
 * @param nbytes_enc - total number of bytes encoded so far; index of 'dest'
 * @param unary      - unary code
 * @param cache      - bitcache
 * @param count      - number of active bits in the 'cache'
 * @param crc        - current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size (unary + cache):
 *	 8/16-bit :    8192u + 7u ==    8199u
 *	   24-bit : 2097152u + 7u == 2097159uL
**/
ALWAYS_INLINE size_t
rice24_write_unary(
	/*@reldef@*/ uint8_t *const RESTRICT dest, rice24_enc unary,
	size_t nbytes_enc, cache64 *const RESTRICT cache,
	bitcnt_enc *const RESTRICT count, crc32_enc *const RESTRICT crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	assert(*count <= (bitcnt_enc) 63u);

	goto loop_entr;
	PRAGMA_NOUNROLL
	do {	unary  -= 32u;
		*cache |= ((cache64) UINT32_MAX) << *count;
		*count |= 0x20u;	/* += 32u; (*count <= 0x07u) */
loop_entr:
		nbytes_enc = rice24_write_cache(
			dest, nbytes_enc, cache, count, crc, WRITECACHE_BYTES
		);
		assert(*count <= (bitcnt_enc) 7u);
	}
	while UNLIKELY ( unary >= (rice24_enc) 32u );

	*cache |= ((cache64) lsmask32((bitcnt) unary)) << *count;
	*count += (bitcnt_enc) (unary + 1u);	/* + terminator */

	assert(*count <= (bitcnt_enc) 39u);
	return nbytes_enc;
}

/**@fn rice24_write_unary_zero
 * @brief write the 'cache' to 'dest' and put a zero unary code in the 'cache'
 *
 * @param dest       - destination buffer
 * @param nbytes_enc - total number of bytes encoded so far; index of 'dest'
 * @param cache      - bitcache
 * @param count      - number of active bits in the 'cache'
 * @param crc        - current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size: 7u
**/
ALWAYS_INLINE size_t
rice24_write_unary_zero(
	/*@reldef@*/ uint8_t *const RESTRICT dest, size_t nbytes_enc,
	cache64 *const RESTRICT cache, bitcnt_enc *const RESTRICT count,
	crc32_enc *const RESTRICT crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	assert(*count <= (bitcnt_enc) 63u);

	nbytes_enc = rice24_write_cache(
		dest, nbytes_enc, cache, count, crc, WRITECACHE_BYTES
	);
	assert(*count <= (bitcnt_enc)  7u);

	*count += 1u;	/* + terminator */

	assert(*count <= (bitcnt_enc)  8u);
	return nbytes_enc;
}

/**@fn rice24_cache_binary
 * @brief write a binary code to the 'cache'
 *
 * @param binary - binary code
 * @param bin_k  - number of bits in the binary code
 * @param cache  - bitcache
 * @param count  - number of active bits in the 'cache'
 *
 * @note max bitcache write size: 3u
**/
ALWAYS_INLINE void
rice24_cache_binary(
	const rice24_enc binary, const bitcnt_enc bin_k,
	cache64 *const RESTRICT cache, bitcnt_enc *const RESTRICT count
)
/*@modifies	*cache,
		*count
@*/
{
	assert(binary <= (rice24_enc) UINT24_MAX);
	assert(bin_k  <= (bitcnt_enc) 24u);
	assert(*count <= (bitcnt_enc) 39u);

	*cache |= ((cache64) binary) << *count;
	*count += bin_k;

	assert(*count <= (bitcnt_enc) 63u);
	return;
}

/**@fn rice24_write_cache
 * @brief write ('*count' / 8u) bytes to 'dest' from the bitcache
 *
 * @param dest       - destination buffer
 * @param nbytes_enc - total number of bytes encoded so far; index of 'dest'
 * @param cache      - bitcache
 * @param count      - number of active bits in the 'cache'
 * @param crc        - current CRC
 * @param mode       - mode of operation; only write full bytes or flush it
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size: 8u
**/
ALWAYS_INLINE size_t
rice24_write_cache(
	/*@reldef@*/ uint8_t *const RESTRICT dest, size_t nbytes_enc,
	cache64 *const RESTRICT cache, bitcnt_enc *const RESTRICT count,
	crc32_enc *const RESTRICT crc, const enum WriteCacheMode mode
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	assert(*count <= (bitcnt_enc) 63u);

	*count += mode;

	PRAGMA_UNROLL(2u)
	while PROBABLE ( *count >= (bitcnt_enc) 8u, 0.9 ){
		dest[nbytes_enc++] = rice24_crc32_enc((uint8_t) *cache, crc);
		*cache >>= 8u;
		*count  -= 8u;
	}

	assert(*count <= (bitcnt_enc)  7u);
	return nbytes_enc;
}

/* ======================================================================== */

/**@fn rice24_decode
 * @brief decode rice codes into a filtered i32 value
 *
 * @param value           - value to decode
 * @param src             - source buffer
 * @param nbytes_dec      - total number of bytes decoded so far; idx of 'src'
 * @param rice            - rice code data for the current channel
 * @param bitcache        - bitcache data
 * @param crc             - current CRC
 * @param unary_lax_limit - limit for the unary code
 *
 * @return number of bytes read from 'src' + 'nbytes_dec'
 *
 * @note max read size (unary + binary):
 *     8/16-bit :    8197u
 *       24-bit : 2097157uL
**/
ALWAYS_INLINE size_t
rice24_decode(
	/*@out@*/ uint32_t *const RESTRICT value,
	const uint8_t *const RESTRICT src, size_t nbytes_dec,
	struct Rice_Dec *const RESTRICT rice,
	struct BitCache_Dec *const RESTRICT bitcache,
	crc32_dec *const RESTRICT crc, const rice24_dec unary_lax_limit
)
/*@modifies	*value,
		*rice,
		*bitcache,
		*crc
@*/
{
	uint32_t   *const RESTRICT sum0  = &rice->sum[0u];
	uint32_t   *const RESTRICT sum1  = &rice->sum[1u];
	bitcnt_dec *const RESTRICT k0    = &rice->k[0u];
	bitcnt_dec *const RESTRICT k1    = &rice->k[1u];
	cache32    *const RESTRICT cache = &bitcache->cache;
	bitcnt_dec *const RESTRICT count = &bitcache->count;
	/* * */
	rice24_dec unary, binary;
	bitcnt_dec bin_k;
	const uint32_t *test0, *test1;

	#define RICE24_DECODE_UNARY(x_unary) { \
		nbytes_dec = rice24_read_unary( \
			(x_unary), src, nbytes_dec, cache, count, crc, \
			unary_lax_limit \
		); \
	}
	#define RICE24_DECODE_BINARY(x_binary) { \
		nbytes_dec = rice24_read_binary( \
			(x_binary), src, nbytes_dec, cache, count, crc, \
			bin_k \
		); \
	}
	#define RICE24_DECODE_STATE_1(x_value) { \
		rice24_update_dec(sum1, k1, (x_value), test1); \
		(x_value) += binexp32((bitcnt) *k0); \
	}
	#define RICE24_DECODE_STATE_0(x_value) { \
		rice24_update_dec(sum0, k0, (x_value), test0); \
	}

	/* unary */
	RICE24_DECODE_UNARY(&unary);
	if PROBABLE ( unary != 0, 0.575 ){
		bin_k  = *k1;
		test1  = &binexp32p4_table[*k1];
		test0  = &binexp32p4_table[*k0];

		/* binary */
		RICE24_DECODE_BINARY(&binary);

		/* value + state */
		*value = (uint32_t) (((unary - 1u) << bin_k) + binary);
		RICE24_DECODE_STATE_1(*value);
		RICE24_DECODE_STATE_0(*value);
	}
	else {	bin_k  = *k0;
		test0  = &binexp32p4_table[*k0];

		/* binary */
		RICE24_DECODE_BINARY(&binary);

		/* value + state */
		*value = (uint32_t) binary;
		RICE24_DECODE_STATE_0(*value);
	}
	return nbytes_dec;
}

/* ------------------------------------------------------------------------ */

/**@fn rice24_read_unary
 * @brief read a unary code from 'src'
 *
 * @param unary      - unary code
 * @param src        - source buffer
 * @param nbytes_dec - total number of bytes decoded so far; index of 'src'
 * @param cache      - bitcache
 * @param count      - number of active bits in the 'cache'
 * @param crc        - current CRC
 * @param lax_limit  - limit for the unary code. has an extra byte of margin,
 *   so if it is surpased, then the data is definitely invalid (corrupt or
 *   malicious). this would be caused by an overly long string of 0xFFu bytes
 *   in the source
 *
 * @return number of bytes read from 'src' + 'nbytes_dec'
 *
 * @note max read size:
 *     8/16-bit :    8194u
 *       24-bit : 2097154uL
**/
ALWAYS_INLINE size_t
rice24_read_unary(
	/*@out@*/ rice24_dec *const RESTRICT unary,
	const uint8_t *const RESTRICT src, size_t nbytes_dec,
	cache32 *const RESTRICT cache, bitcnt_dec *const RESTRICT count,
	crc32_dec *const RESTRICT crc, const rice24_dec lax_limit
)
/*@modifies	*unary,
		*cache,
		*count,
		*crc
@*/
{
	bitcnt_dec nbits;
	uint8_t inbyte;

	assert(*count <= (bitcnt_dec) 7u);

	nbits  = (bitcnt_dec) TBCNT8(*cache);
	*unary = (rice24_dec) nbits;
	if IMPROBABLE ( nbits == *count, 0.25 ){
		PRAGMA_NOUNROLL
		do {	inbyte  = rice24_crc32_dec(src[nbytes_dec++], crc);
			nbits   = (bitcnt_dec) TBCNT8(inbyte);
			*unary += nbits;
			if UNLIKELY ( *unary > lax_limit ){
				nbits = 0; /* prevents *count underflow */
				break;
			}
		} while UNLIKELY ( nbits == (bitcnt_dec) 8u );

		*cache = (cache32) inbyte;
		*count = (bitcnt_dec) 8u;
	}
	*cache >>= nbits + 1u;	/* + terminator */
	*count  -= nbits + 1u;	/* ~            */

	assert(*count <= (bitcnt_dec) 7u);
	return nbytes_dec;
}

/**@fn rice24_read_binary
 * @brief read a binary code from 'src'
 *
 * @param binary     - binary code
 * @param src        - source buffer
 * @param nbytes_dec - total number of bytes decoded so far; index of 'src'
 * @param cache      - bitcache
 * @param count      - number of active bits in the 'cache'
 * @param crc        - current CRC
 * @param bin_k      - number of bits in the binary code
 *
 * @return number of bytes read from 'src' + 'nbytes_dec'
 *
 * @note max read size: 3u
 *   a check like in the unary reader is unnecessary
**/
ALWAYS_INLINE size_t
rice24_read_binary(
	/*@out@*/ rice24_dec *const RESTRICT binary,
	const uint8_t *const RESTRICT src, size_t nbytes_dec,
	cache32 *const RESTRICT cache, bitcnt_dec *const RESTRICT count,
	crc32_dec *const RESTRICT crc, const bitcnt_dec bin_k
)
/*@modifies	*binary,
		*cache,
		*count,
		*crc
@*/
{
	uint8_t inbyte;

	assert(bin_k   <= (bitcnt_dec) 24u);
	assert(*count  <= (bitcnt_dec)  7u);

	PRAGMA_NOUNROLL
	while PROBABLE ( *count < bin_k, 0.9 ){
		inbyte  = rice24_crc32_dec(src[nbytes_dec++], crc);
		*cache |= ((cache32) inbyte) << *count;
		*count += 8u;
	}
	*binary  = (rice24_dec) BZHI32(*cache, bin_k);
	*cache >>= bin_k;
	*count  -= bin_k;

	assert(*binary <= (rice24_dec) UINT24_MAX);
	assert(*count  <= (bitcnt_dec)  7u);
	return nbytes_dec;
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_RICE24_H */
