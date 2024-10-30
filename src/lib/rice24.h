#ifndef TTA_CODEC_RICE_H
#define TTA_CODEC_RICE_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/rice.h                                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <limits.h>	// tbcnt8_32 assert
#include <stddef.h>	// size_t

#include "../bits.h"
#include "../splint.h"

#include "common.h"
#include "crc32.h"

//////////////////////////////////////////////////////////////////////////////

// the lax_limit has an extra byte to make handling invalid data faster/easier
#define UNARY_LAX_LIMIT_1_2	((rice24_dec) ((8u *    8194u)  - 1u))
#define UNARY_LAX_LIMIT_3	((rice24_dec) ((8u * 2097154uL) - 1u))

// max unary + binary r/w size for one value
#define RICE_ENC_MAX_1_2	((size_t)    8200u)
#define RICE_ENC_MAX_3		((size_t) 2097160uL)
#define RICE_DEC_MAX_1_2	((size_t)    8197u)
#define RICE_DEC_MAX_3		((size_t) 2097157uL)

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 binexp32p4_table[26u];

#ifdef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 lsmask32_table[32u];
#endif

#ifdef TBCNT8_TABLE
/*@unchecked@*/ /*@unused@*/
extern HIDDEN const  u8 tbcnt8_table[256u];
#endif

//////////////////////////////////////////////////////////////////////////////

INLINE CONST rice24_dec get_unary_lax_limit(enum TTASampleBytes) /*@*/;

#ifndef NDEBUG
INLINE CONST size_t get_rice24_enc_max(enum TTASampleBytes) /*@*/;
INLINE CONST size_t get_rice24_dec_max(enum TTASampleBytes) /*@*/;
#endif

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST u32 binexp32(bitcnt) /*@*/;
ALWAYS_INLINE CONST u32 lsmask32(bitcnt) /*@*/;

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
ALWAYS_INLINE void rice24_update(
	u32 *restrict sum, bitcnt *restrict k, u32, const u32 *restrict
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
	/*@partial@*/ u8 *restrict dest, u32, size_t,
	struct Rice *restrict rice, struct BitCache_Enc *restrict bitcache,
	crc32_enc *restrict crc
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
INLINE size_t rice24_encode_cacheflush(
	/*@partial@*/ u8 *restrict dest, size_t,
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
	/*@partial@*/ u8 *restrict dest, rice24_enc, size_t,
	cache64 *restrict cache, bitcnt *restrict count,
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
	/*@partial@*/ u8 *const restrict dest, size_t,
	cache64 *const restrict cache, bitcnt *const restrict count,
	crc32_enc *const restrict crc
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
	rice24_enc, cache64 *restrict cache, bitcnt *restrict count, bitcnt
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
	/*@partial@*/ u8 *restrict dest, size_t, cache64 *restrict cache,
	bitcnt *restrict count, crc32_enc *restrict crc
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
	struct Rice *restrict rice, struct BitCache_Dec *restrict bitcache,
	crc32_dec *restrict crc, rice24_dec
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
	cache32 *restrict cache, bitcnt *restrict count,
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
	cache32 *restrict cache, bitcnt *restrict count,
	crc32_dec *restrict crc, bitcnt
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
INLINE CONST rice24_dec
get_unary_lax_limit(const enum TTASampleBytes samplebytes)
/*@*/
{
	u32 r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_2:
		r = UNARY_LAX_LIMIT_1_2;
		break;
	case TTASAMPLEBYTES_3:
		r = UNARY_LAX_LIMIT_3;
		break;
	}
	return r;
}

#ifndef NDEBUG
/**@fn get_rice24_enc_max
 * @brief max number of bytes rice24_encode could write
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return max number of bytes rice24_encode could write
**/
INLINE CONST size_t
get_rice24_enc_max(const enum TTASampleBytes samplebytes)
/*@*/
{
	size_t r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_2:
		r = RICE_ENC_MAX_1_2;
		break;
	case TTASAMPLEBYTES_3:
		r = RICE_ENC_MAX_3;
		break;
	}
	return r;
}

/**@fn get_rice24_dec_max
 * @brief max number of bytes rice24_decode could read
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return max number of bytes rice24_decode could read
**/
INLINE CONST size_t
get_rice24_dec_max(const enum TTASampleBytes samplebytes)
/*@*/
{
	size_t r;
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
	case TTASAMPLEBYTES_2:
		r = RICE_DEC_MAX_1_2;
		break;
	case TTASAMPLEBYTES_3:
		r = RICE_DEC_MAX_3;
		break;
	}
	return r;
}
#endif

//==========================================================================//

/**@fn binexp32
 * @brief binary exponentiation 32-bit (2**'k')
 *
 * @param k bit number
 *
 * @return a 32-bit mask with only the 'k'th bit set
 *
 * @pre k <= (bitcnt) 31u	// 24u in practice
**/
ALWAYS_INLINE CONST u32
binexp32(const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 31u);

	return (((u32) 0x1u) << k);
}

/**@fn lsmask32
 * @brief least significant mask 32-bit
 *
 * @param k number of bits in the mask
 *
 * @return a mask with 'k' low bits set
 *
 * @pre k <= (bitcnt) 31u
 *
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
ALWAYS_INLINE CONST u32
lsmask32(const bitcnt k)
/*@*/
{
	assert(k <= (bitcnt) 31u);

#ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
	return ((((u32) 0x1u) << k) - 1u);
#else
	return lsmask32_table[k];
#endif
}

/**@fn TBCNT8
 * @brief trailing bit count 8-bit
 *   macro'd because fast types were causing extra casts with inline functions
 *
 * @param x value to count
 *
 * @return number of trailing bits
 *
 * @note 'x' will never be UINT[32|64]_MAX
**/
#ifndef TBCNT8_TABLE
#if HAS_BUILTIN(BUILTIN_TZCNT32)
#define TBCNT8(x)	((bitcnt) BUILTIN_TZCNT32(~((u32) (x))))
#elif HAS_BUILTIN(BUILTIN_TZCNT64)
#define TBCNT8(x)	((bitcnt) BUILTIN_TZCNT64(~((u64) (x))))
#else
#error "TBCNT8"
#endif
#else // defined(TBCNT8_TABLE)
#define TBCNT8(x)	((bitcnt) tbcnt8_table[(x)])
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
 * @param x input byte
 * @param crc[in out] current CRC
 *
 * @return 'x'
**/
ALWAYS_INLINE u8
rice24_crc32_dec(const u8 x, crc32_dec *const restrict crc)
/*@modifies	*crc@*/
{
	*crc = crc32_cont_dec(x, *crc);
	return x;
}

//==========================================================================//

/**@fn rice24_update
 * @brief update the rice state
 *
 * @param sum[in out] rice->sum[]
 * @param k[in out] rice->k[]
 * @param value input value to code
 * @param test[in] binexp32p4 comparison values
 *
 * @pre  *k <= (bitcnt) 24u
 * @post *k <= (bitcnt) 24u
**/
ALWAYS_INLINE void
rice24_update(
	u32 *const restrict sum, bitcnt *const restrict k, const u32 value,
	const u32 *const restrict test
)
/*@modifies	*sum,
		*k
@*/
{
	assert(*k <= (bitcnt) 24u);

	*sum += value - (*sum >> 4u);	// may wrap
	if IMPROBABLE ( *sum < test[0u], 0.027 ){
		*k -= 1u;
	}
	else if IMPROBABLE ( *sum > test[1u], 0.027 ){
		*k += 1u;
	} else{;}

	assert(*k <= (bitcnt) 24u);
	return;
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
 *     8/16-bit :    8200u
 *       24-bit : 2097160uL
 * @note affected by LIBTTAr_OPT_NO_FAT_RICE_ENCODER
**/
ALWAYS_INLINE size_t
rice24_encode(
	/*@partial@*/ u8 *const restrict dest, u32 value,
	size_t nbytes_enc, struct Rice *const restrict rice,
	struct BitCache_Enc *const restrict bitcache,
	crc32_enc *const restrict crc
)
/*@modifies	*dest,
		*rice,
		*bitcache,
		*crc
@*/
{
	u32     *const restrict sum0  = &rice->sum[0u];
	u32     *const restrict sum1  = &rice->sum[1u];
	bitcnt  *const restrict k0    = &rice->k[0u];
	bitcnt  *const restrict k1    = &rice->k[1u];
	cache64 *const restrict cache = &bitcache->cache;
	bitcnt  *const restrict count = &bitcache->count;

	rice24_enc unary, binary;
	bitcnt bin_k;
	const u32 *test0, *test1;

	#define RICE_ENCODE_STATE_0(Xvalue) { \
		bin_k      = *k0; \
		test0      = &binexp32p4_table[*k0]; \
		rice24_update(sum0, k0, (Xvalue), test0); \
	}
	#define RICE_ENCODE_STATE_1(Xvalue) { \
		(Xvalue)  -= binexp32(bin_k); \
		bin_k      = *k1; \
		test1      = &binexp32p4_table[*k1]; \
		rice24_update(sum1, k1, (Xvalue), test1); \
	}
	#define RICE_ENCODE_UNARY(Xunary) { \
		unary      = (Xunary); \
		nbytes_enc = rice24_write_unary( \
			dest, unary, nbytes_enc, cache, count, crc \
		); \
	}
	#define RICE_ENCODE_UNARY_ZERO { \
		nbytes_enc = rice24_write_unary_zero( \
			dest, nbytes_enc, cache, count, crc \
		); \
	}
	#define RICE_ENCODE_BINARY(Xbinary) { \
		binary     = (Xbinary); \
		rice24_cache_binary(binary, cache, count, bin_k); \
	}

#ifndef LIBTTAr_OPT_NO_FAT_RICE_ENCODER
	// value + state
	RICE_ENCODE_STATE_0(value);
	if PROBABLE ( value >= binexp32(bin_k), 0.575 ){
		RICE_ENCODE_STATE_1(value);

		// unary + binary
		RICE_ENCODE_UNARY((rice24_enc) ((value >> bin_k) + 1u));
		RICE_ENCODE_BINARY((rice24_enc) (value & lsmask32(bin_k)));
	}
	else {	// unary-zero + binary
		RICE_ENCODE_UNARY_ZERO;
		RICE_ENCODE_BINARY((rice24_enc) (value & lsmask32(bin_k)));
	}
#else
	// value + state
	unary = 0;
	RICE_ENCODE_STATE_0(value);
	if PROBABLE ( value >= binexp32(bin_k), 0.575 ){
		RICE_ENCODE_STATE_1(value);
		unary = (rice24_enc) ((value >> bin_k) + 1u);
	}

	// unary + binary
	RICE_ENCODE_UNARY(unary);
	RICE_ENCODE_BINARY((rice24_enc) (value & lsmask32(bin_k)));
#endif
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
 * @pre bitcache->count <= (bitcnt) 64u	// 63 in practice
 *
 * @note max write size: 8u
**/
INLINE size_t
rice24_encode_cacheflush(
	/*@partial@*/ u8 *const restrict dest, size_t nbytes_enc,
	struct BitCache_Enc *const restrict bitcache,
	u32 *const restrict crc_inout
)
/*@modifies	*dest,
		*bitcache,
		*crc_inout
@*/
{
	cache64 *const restrict cache = &bitcache->cache;
	bitcnt  *const restrict count = &bitcache->count;
	crc32_enc crc = (crc32_enc) *crc_inout;

	assert(*count <= (bitcnt) 64u);

	*count += 7u;
	nbytes_enc = rice24_write_cache(dest, nbytes_enc, cache, count, &crc);
	*count  = 0u;

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
 * @pre  *count <= (bitcnt) 64u	// 63 in practice
 * @post *count <= (bitcnt) 40u	// 39u in practice
 *
 * @note max write size (unary + cache):
 *	 8/16-bit :    8192u + 8u ==    8200u
 *	   24-bit : 2097152u + 8u == 2097160uL
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
ALWAYS_INLINE size_t
rice24_write_unary(
	/*@partial@*/ u8 *const restrict dest, rice24_enc unary,
	size_t nbytes_enc, cache64 *const restrict cache,
	bitcnt *const restrict count, crc32_enc *const restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	assert(*count <= (bitcnt) 64u);

	goto loop_entr;
	do {	unary  -= 32u;
		*cache |= ((cache64) 0xFFFFFFFFu) << *count;
		*count += 32u;
loop_entr:
		nbytes_enc = rice24_write_cache(
			dest, nbytes_enc, cache, count, crc
		);
	} while UNLIKELY ( unary >= (rice24_dec) 32u );

	*cache |= ((cache64) lsmask32((bitcnt) unary)) << *count;
	*count += (bitcnt) (unary + 1u);	// + terminator

	assert(*count <= (bitcnt) 40u);
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
 * @pre  *count <= (bitcnt) 64u	// 63 in practice
 * @post *count <= (bitcnt)  8u
 *
 * @note max write size: 8u
**/
ALWAYS_INLINE size_t
rice24_write_unary_zero(
	/*@partial@*/ u8 *const restrict dest, size_t nbytes_enc,
	cache64 *const restrict cache, bitcnt *const restrict count,
	crc32_enc *const restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	assert(*count <= (bitcnt) 64u);

	nbytes_enc = rice24_write_cache(dest, nbytes_enc, cache, count, crc);
	*count += 1u;	// + terminator

	assert(*count <= (bitcnt)  8u);
	return nbytes_enc;
}

/**@fn rice24_cache_binary
 * @brief write a binary code to the 'cache'
 *
 * @param binary the binary code
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param bin_k number of bits in the binary code
 *
 * @pre  *count <= (bitcnt) 40u	// 39u in practice
 * @post *count <= (bitcnt) 64u	// 63 in practice
 *
 * @note max bitcache write size: 3u
**/
ALWAYS_INLINE void
rice24_cache_binary(
	const rice24_enc binary, cache64 *const restrict cache,
	bitcnt *const restrict count, const bitcnt bin_k
)
/*@modifies	*cache,
		*count
@*/
{
	assert(*count <= (bitcnt) 40u);

	*cache |= ((cache64) binary) << *count;
	*count += bin_k;

	assert(*count <= (bitcnt) 64u);
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
 * @pre  *count <= (bitcnt) 71u	// 70u in practice (rice24_encode_cacheflush)
 * @post *count <= (bitcnt)  7u
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size: 8u
 * @note affected by LIBTTAr_OPT_NO_FAT_RICE_ENCODER
**/
ALWAYS_INLINE size_t
rice24_write_cache(
	/*@partial@*/ u8 *const restrict dest, size_t nbytes_enc,
	cache64 *const restrict cache, bitcnt *const restrict count,
	crc32_enc *const restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	#define RICE_WRITE_CACHE(Xcache) { \
		dest[nbytes_enc++] = rice24_crc32_enc((u8) (Xcache), crc); \
		(Xcache) >>= 8u; \
		*count    -= 8u; \
	}

	assert(*count <= (bitcnt) 71u);

#ifndef LIBTTAr_OPT_NO_FAT_RICE_ENCODER
	if PROBABLE ( *count >= (bitcnt) 8u, 0.9 ){
		do {	RICE_WRITE_CACHE(*cache);
		} while ( *count >= (bitcnt) 8u );
	}
#else
	while PROBABLE ( *count >= (bitcnt) 8u, 0.9 ){
		RICE_WRITE_CACHE(*cache);
	}
#endif
	assert(*count <= (bitcnt)  7u);
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
 * @note affected by LIBTTAr_OPT_NO_FAT_RICE_DECODER
**/
ALWAYS_INLINE size_t
rice24_decode(
	/*@out@*/ u32 *const restrict value, const u8 *const restrict src,
	size_t nbytes_dec, struct Rice *const restrict rice,
	struct BitCache_Dec *const restrict bitcache,
	crc32_dec *const restrict crc, const rice24_dec unary_lax_limit
)
/*@modifies	*value,
		*rice,
		*bitcache,
		*crc
@*/
{
	u32     *const restrict sum0  = &rice->sum[0u];
	u32     *const restrict sum1  = &rice->sum[1u];
	bitcnt  *const restrict k0    = &rice->k[0u];
	bitcnt  *const restrict k1    = &rice->k[1u];
	cache32 *const restrict cache = &bitcache->cache;
	bitcnt  *const restrict count = &bitcache->count;

	rice24_dec unary, binary;
	bitcnt bin_k;
	const u32 *test0, *test1;

	#define RICE_DECODE_UNARY(Xunary) { \
		nbytes_dec = rice24_read_unary( \
			(Xunary), src, nbytes_dec, cache, count, crc, \
			unary_lax_limit \
		); \
	}
	#define RICE_DECODE_BINARY(Xbinary) { \
		nbytes_dec = rice24_read_binary( \
			(Xbinary), src, nbytes_dec, cache, count, crc, bin_k \
		); \
	}
	#define RICE_DECODE_STATE_1(Xvalue) { \
		rice24_update(sum1, k1, (Xvalue), test1); \
		(Xvalue)  += binexp32(*k0); \
	}
	#define RICE_DECODE_STATE_0(Xvalue) { \
		rice24_update(sum0, k0, (Xvalue), test0); \
	}

#ifndef LIBTTAr_OPT_NO_FAT_RICE_DECODER
	// unary
	RICE_DECODE_UNARY(&unary);
	if PROBABLE ( unary != 0, 0.575 ){
		bin_k   = *k1;
		test1   = &binexp32p4_table[*k1];
		test0   = &binexp32p4_table[*k0];

		// binary
		RICE_DECODE_BINARY(&binary);

		// value + state
		*value  = (((u32) (unary - 1u)) << bin_k) + ((u32) binary);
		RICE_DECODE_STATE_1(*value);
		RICE_DECODE_STATE_0(*value);
	}
	else {	bin_k   = *k0;
		test0   = &binexp32p4_table[*k0];

		// binary
		RICE_DECODE_BINARY(&binary);

		// value + state
		*value  = (u32) binary;
		RICE_DECODE_STATE_0(*value);
	}
#else
	// unary
	test1  = NULL;
	RICE_DECODE_UNARY(&unary);
	if PROBABLE ( unary != 0, 0.575 ){
		unary  -= 1u;
		bin_k   = *k1;
		test1   = &binexp32p4_table[*k1];
	}
	else {	bin_k   = *k0; }
	test0  = &binexp32p4_table[*k0];

	// binary
	RICE_DECODE_BINARY(&binary);

	// value + state
	*value = (((u32) unary) << bin_k) + ((u32) binary);
	if PROBABLE ( test1 != NULL, 0.575 ){
		RICE_DECODE_STATE_1(*value);
	}
	RICE_DECODE_STATE_0(*value);
#endif
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
 * @pre  *count <= (bitcnt) 8u	// 7u in practice
 * @post *count <= (bitcnt) 7u
 *
 * @note max read size:
 *     8/16-bit :    8194u
 *       24-bit : 2097154uL
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
ALWAYS_INLINE size_t
rice24_read_unary(
	/*@out@*/ rice24_dec *const restrict unary,
	const u8 *const restrict src, size_t nbytes_dec,
	cache32 *const restrict cache, bitcnt *const restrict count,
	crc32_dec *const restrict crc, const rice24_dec lax_limit
)
/*@modifies	*unary,
		*cache,
		*count,
		*crc
@*/
{
	bitcnt nbit;
	u8 inbyte;

	assert(*count <= (bitcnt) 8u);

	nbit   = TBCNT8(*cache);
	*unary = (rice24_dec) nbit;
	if IMPROBABLE ( nbit == *count, 0.25 ){
		do {	inbyte  = rice24_crc32_dec(src[nbytes_dec++], crc);
			*cache  = (cache32) inbyte;
			nbit    = TBCNT8(*cache);
			*unary += nbit;
			if UNLIKELY ( *unary > lax_limit ){
				nbit = 0;	// prevents *count underflow
				break;
			}
		} while UNLIKELY ( nbit == (bitcnt) 8u );
		*count = (bitcnt) 8u;
	}
	*cache >>= nbit + 1u;	// + terminator
	*count  -= nbit + 1u;	// ~

	assert(*count <= (bitcnt) 7u);
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
 * @pre  *count <= (bitcnt) 8u	// 7u in practice
 * @post *count <= (bitcnt) 8u	// ~
 *
 * @note max read size: 3u
 *   a check like in the unary reader is unnecessary
**/
ALWAYS_INLINE size_t
rice24_read_binary(
	/*@out@*/ rice24_dec *const restrict binary,
	const u8 *const restrict src, size_t nbytes_dec,
	cache32 *const restrict cache, bitcnt *const restrict count,
	crc32_dec *const restrict crc, const bitcnt bin_k
)
/*@modifies	*binary,
		*cache,
		*count,
		*crc
@*/
{
	u8 inbyte;

	assert(*count <= (bitcnt) 8u);

	while PROBABLE ( *count < bin_k, 0.9 ){
		inbyte  = rice24_crc32_dec(src[nbytes_dec++], crc);
		*cache |= ((cache32) inbyte) << *count;
		*count += 8u;
	}
	*binary  = (rice24_dec) (*cache & lsmask32(bin_k));
	*cache >>= bin_k;
	*count  -= bin_k;

	assert(*count <= (bitcnt) 8u);
	return nbytes_dec;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
