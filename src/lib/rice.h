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

enum ShiftMaskMode {
	SMM_CONST,
	SMM_SHIFT,
	SMM_TABLE
};

#ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
#define SMM_ENC		SMM_SHIFT
#else
#define SMM_ENC		SMM_TABLE
#endif

#define SMM_DEC		SMM_TABLE

//////////////////////////////////////////////////////////////////////////////

// max unary size:
//	8/16-bit :   32u bytes + 7u bits + terminator
//	  24-bit : 4096u bytes + 7u bits + terminator
// the lax_limit has an extra byte to make handling invalid data faster/easier
#define UNARY_LAX_LIMIT_1_2	((rice24) ((8u *   34u) - 1u))
#define UNARY_LAX_LIMIT_3	((rice24) ((8u * 4098u) - 1u))

// max unary + binary r/w size for one value
#define RICE_ENC_MAX_1_2	((size_t)   40u)
#define RICE_ENC_MAX_3		((size_t) 4104u)
#define RICE_DEC_MAX_1_2	((size_t)   37u)
#define RICE_DEC_MAX_3		((size_t) 4101u)

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 binexp32p4_table[26u];
/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 lsmask32_table[32u];

#if TBCNT8_TEST
/*@unchecked@*/ /*@unused@*/
extern HIDDEN const  u8 tbcnt8_table[256u];
#endif

//////////////////////////////////////////////////////////////////////////////

INLINE CONST rice24 get_unary_lax_limit(enum TTASampleBytes) /*@*/;

#ifndef NDEBUG
INLINE CONST size_t get_rice_enc_max(enum TTASampleBytes) /*@*/;
INLINE CONST size_t get_rice_dec_max(enum TTASampleBytes) /*@*/;
#endif

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST u32 binexp32(bitcnt) /*@*/;
ALWAYS_INLINE CONST u32 lsmask32(bitcnt, enum ShiftMaskMode) /*@*/;

//--------------------------------------------------------------------------//

#ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
ALWAYS_INLINE CONST bitcnt tbcnt8_32(u32) /*@*/;
#endif

#if TBCNT8_TEST
ALWAYS_INLINE CONST bitcnt tbcnt8(u8) /*@*/;
#endif

//--------------------------------------------------------------------------//

#undef sum
#undef k
ALWAYS_INLINE void rice_update_enc(
	u32 *restrict sum, bitcnt *restrict k, u32, const u32 *restrict
)
/*@modifies	*sum,
		*k
@*/
;

#undef sum
#undef k
ALWAYS_INLINE void rice_update_dec(
	u32 *restrict sum, bitcnt *restrict k, u32, const u32 *restrict
)
/*@modifies	*sum,
		*k
@*/
;
//--------------------------------------------------------------------------//

#undef crc
ALWAYS_INLINE u8 rice_crc32(u8, u32 *restrict crc)
/*@modifies	*crc@*/
;

//--------------------------------------------------------------------------//

#undef dest
#undef rice
#undef bitcache
#undef crc
ALWAYS_INLINE size_t rice_encode(
	/*@partial@*/ u8 *restrict dest, u32, size_t,
	struct Rice *restrict rice, struct BitCache *restrict bitcache,
	u32 *restrict crc
)
/*@modifies	*dest,
		*rice,
		*bitcache,
		*crc
@*/
;

#undef dest
#undef cache
#undef count
#undef crc
INLINE size_t rice_encode_cacheflush(
	/*@partial@*/ u8 *restrict dest, size_t,
	struct BitCache *restrict bitcache, u32 *restrict crc
)
/*@modifies	*dest,
		*bitcache,
		*crc
@*/
;

#undef dest
#undef cache
#undef count
#undef crc
ALWAYS_INLINE size_t rice_write_unary(
	/*@partial@*/ u8 *restrict dest, rice24, size_t,
	cache64 *restrict cache, bitcnt *restrict count, u32 *restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
;

#undef cache
#undef count
ALWAYS_INLINE void rice_cache_binary(
	rice24, cache64 *restrict cache, bitcnt *restrict count, bitcnt
)
/*@modifies	*cache,
		*count
@*/
;

#undef dest
#undef cache
#undef count
#undef crc
ALWAYS_INLINE size_t rice_write_cache(
	/*@partial@*/ u8 *restrict dest, size_t, cache64 *restrict cache,
	bitcnt *restrict count, u32 *restrict crc
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
ALWAYS_INLINE size_t rice_decode(
	/*@out@*/ u32 *restrict value, const u8 *restrict, size_t,
	struct Rice *restrict rice, struct BitCache *restrict bitcache,
	u32 *restrict crc, rice24
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
ALWAYS_INLINE size_t rice_read_unary(
	/*@out@*/ rice24 *restrict unary, const u8 *restrict, size_t,
	cache32 *restrict cache, bitcnt *restrict count, u32 *restrict crc,
	rice24
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
ALWAYS_INLINE size_t rice_read_binary(
	/*@out@*/ rice24 *restrict binary, const u8 *restrict, size_t,
	cache32 *restrict cache, bitcnt *restrict count, u32 *restrict crc,
	bitcnt
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
INLINE CONST rice24
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
/**@fn get_rice_enc_max
 * @brief max number of bytes rice_encode could write
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return max number of bytes rice_encode could write
**/
INLINE CONST size_t
get_rice_enc_max(const enum TTASampleBytes samplebytes)
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

/**@fn get_rice_dec_max
 * @brief max number of bytes rice_decode could read
 *
 * @param samplebytes number of bytes per PCM sample
 *
 * @return max number of bytes rice_decode could read
**/
INLINE CONST size_t
get_rice_dec_max(const enum TTASampleBytes samplebytes)
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
 * @param mode constant, shift, or lookup table
 *
 * @return a mask with 'k' low bits set
 *
 * @pre k <= (bitcnt) 31u
**/
ALWAYS_INLINE CONST u32
lsmask32(const bitcnt k, const enum ShiftMaskMode mode)
/*@*/
{
	assert(k <= (bitcnt) 31u);

	u32 r;
	switch ( mode ){
	case SMM_CONST:
	case SMM_SHIFT:
		r = ((((u32) 0x1u) << k) - 1u);
		break;
	case SMM_TABLE:
		r = lsmask32_table[k];
		break;
	}
	return r;
}

//==========================================================================//

#ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
#define TBCNT8(cache) 	(tbcnt8_32((u32) (cache)))
#else
#define TBCNT8(cache) 	(tbcnt8((u8) (cache)))
#endif

//--------------------------------------------------------------------------//

#ifndef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
/**@fn tbcnt8_32
 * @brief trailing bit count 8-bit (32-bit version)
 *
 * @param x value to count
 *
 * @return number of trailing bits
 *
 * @pre x != UINT32_MAX
**/
ALWAYS_INLINE CONST bitcnt
tbcnt8_32(const u32 x)
/*@*/
{
	assert(x != UINT32_MAX);

#if HAS_BUILTIN(BUILTIN_TZCNT32)
	return (bitcnt) BUILTIN_TZCNT32(~x);
#elif HAS_BUILTIN(BUILTIN_TZCNT64)
	return (bitcnt) BUILTIN_TZCNT64((u64) ~x);
#else
	return (bitcnt) tbcnt8((u8) x);
#endif
}
#endif

#if TBCNT8_TEST
/**@fn tbcnt8
 * @brief trailing bit count 8-bit
 *
 * @param x value to count
 *
 * @return number of trailing bits
 *
 * @note defined for all values
**/
ALWAYS_INLINE CONST bitcnt
tbcnt8(const u8 x)
/*@*/
{
	return (bitcnt) tbcnt8_table[x];
}
#endif

//==========================================================================//

/**@fn rice_update_enc
 * @brief update the rice state; encode version
 *
 * @param sum[in out] rice->sum[]
 * @param k[in out] rice->k[]
 * @param value input value to code
 * @param test_in[in] binexp32p4 comparison values
 *
 * @pre  *k <= (bitcnt) 24u
 * @post *k <= (bitcnt) 24u
**/
ALWAYS_INLINE void
rice_update_enc(
	u32 *const restrict sum, bitcnt *const restrict k, const u32 value,
	const u32 *const restrict test_in
)
/*@modifies	*sum,
		*k
@*/
{
	u32 test[2u];

	assert(*k <= (bitcnt) 24u);

	MEMCPY(test, test_in, sizeof test);

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

/**@fn rice_update_dec
 * @brief update the rice state; decode version
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
rice_update_dec(
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

/**@fn rice_crc32
 * @brief add a byte to a CRC
 *
 * @param x input byte
 * @param crc[in out] current CRC
 *
 * @return 'x'
**/
ALWAYS_INLINE u8
rice_crc32(const u8 x, u32 *const restrict crc)
/*@modifies	*crc@*/
{
	*crc = crc32_cont(x, *crc);
	return x;
}

//==========================================================================//

/**@fn rice_encode
 * @brief encode a filtered i32 value into rice codes
 *
 * @param dest[out] destination buffer
 * @param nbytes_enc total number of bytes encoded so far; index of 'dest'
 * @param value value to encode
 * @param rice[in out] the rice code data for the current channel
 * @param bitcache[in out] the bitcache data
 * @param crc[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size (unary + binary):
 *     8/16-bit :   40u
 *       24-bit : 4104u
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
ALWAYS_INLINE size_t
rice_encode(
	/*@partial@*/ u8 *const restrict dest, u32 value,
	size_t nbytes_enc, struct Rice *const restrict rice,
	struct BitCache *const restrict bitcache, u32 *const restrict crc
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
	cache64 *const restrict cache = &bitcache->cache.enc;
	bitcnt  *const restrict count = &bitcache->count;

	rice24 unary = 0, binary;
	bitcnt bin_k;
	const u32 *test0, *test1;

	// value + state
	bin_k = *k0;
	test0 = &binexp32p4_table[*k0];
	rice_update_enc(sum0, k0, value, test0);
	if PROBABLE ( value >= binexp32(bin_k), 0.575 ){
		value -= binexp32(bin_k);
		bin_k  = *k1;
		test1  = &binexp32p4_table[*k1];
		rice_update_enc(sum1, k1, value, test1);
		unary  = (rice24) ((value >> bin_k) + 1u);
	}

	// unary
	nbytes_enc = rice_write_unary(
		dest, unary, nbytes_enc, cache, count, crc
	);

	// binary
	binary = (rice24) (value & lsmask32(bin_k, SMM_ENC));
	rice_cache_binary(binary, cache, count, bin_k);

	return nbytes_enc;
}

/**@fn rice_encode_cacheflush
 * @brief flush any data left in the 'bitcache' to 'dest'
 *
 * @param dest[out] destination buffer
 * @param nbytes_enc total number of bytes encoded so far; index of 'dest'
 * @param bitcache[in out] the bitcache data
 * @param crc[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @pre bitcache->count <= (bitcnt) 64u	// 63 in practice
 *
 * @note max write size: 8u
**/
INLINE size_t
rice_encode_cacheflush(
	/*@partial@*/ u8 *const restrict dest, size_t nbytes_enc,
	struct BitCache *const restrict bitcache, u32 *const restrict crc
)
/*@modifies	*dest,
		*bitcache,
		*crc
@*/
{
	cache64 *const restrict cache = &bitcache->cache.enc;
	bitcnt  *const restrict count = &bitcache->count;

	assert(*count <= (bitcnt) 64u);

	*count += 7u;
	nbytes_enc = rice_write_cache(dest, nbytes_enc, cache, count, crc);
	*count  = 0u;
	return nbytes_enc;
}

//--------------------------------------------------------------------------//

/**@fn rice_write_unary
 * @brief write a unary code to 'dest'
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
 *	 8/16-bit :   32u + 8u ==   40u
 *	   24-bit : 4096u + 8u == 4104u
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
ALWAYS_INLINE size_t
rice_write_unary(
	/*@partial@*/ u8 *const restrict dest, rice24 unary,
	size_t nbytes_enc, cache64 *const restrict cache,
	bitcnt *const restrict count, u32 *const restrict crc
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
		nbytes_enc = rice_write_cache(
			dest, nbytes_enc, cache, count, crc
		);
	} while UNLIKELY ( unary >= (rice24) 32u );

	*cache |= ((cache64) lsmask32((bitcnt) unary, SMM_ENC)) << *count;
	*count += (bitcnt) (unary + 1u);	// + terminator

	assert(*count <= (bitcnt) 40u);
	return nbytes_enc;
}

/**@fn rice_cache_binary
 * @brief write a binary code to the bitcache
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
rice_cache_binary(
	const rice24 binary, cache64 *const restrict cache,
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

/**@fn rice_write_cache
 * @brief write ('*count' / 8u) bytes to 'dest' from the bitcache
 *
 * @param dest[out] destination buffer
 * @param nbytes_enc total number of bytes encoded so far; index of 'dest'
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 *
 * @pre  *count <= (bitcnt) 71u	// 70u in practice (rice_encode_cacheflush)
 * @post *count <= (bitcnt)  7u
 *
 * @return number of bytes written to 'dest' + 'nbytes_enc'
 *
 * @note max write size: 8u
**/
ALWAYS_INLINE size_t
rice_write_cache(
	/*@partial@*/ u8 *const restrict dest, size_t nbytes_enc,
	cache64 *const restrict cache, bitcnt *const restrict count,
	u32 *const restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	assert(*count <= (bitcnt) 71u);

	while ( *count >= (bitcnt) 8u ){
		dest[nbytes_enc++] = rice_crc32((u8) *cache, crc);
		*cache >>= 8u;
		*count  -= 8u;
	}

	assert(*count <= (bitcnt)  7u);
	return nbytes_enc;
}

//==========================================================================//

/**@fn rice_decode
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
 *     8/16-bit :   37u
 *       24-bit : 4101u
**/
ALWAYS_INLINE size_t
rice_decode(
	/*@out@*/ u32 *const restrict value, const u8 *const restrict src,
	size_t nbytes_dec, struct Rice *const restrict rice,
	struct BitCache *const restrict bitcache, u32 *const restrict crc,
	const rice24 unary_lax_limit
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
	cache32 *const restrict cache = &bitcache->cache.dec;
	bitcnt  *const restrict count = &bitcache->count;

	rice24 unary, binary;
	bitcnt bin_k;
	const u32 *test0, *test1 = NULL;

	// unary
	nbytes_dec = rice_read_unary(
		&unary, src, nbytes_dec, cache, count, crc, unary_lax_limit
	);
	if PROBABLE ( unary != 0, 0.575 ){
		unary  -= 1u;
		bin_k   = *k1;
		test1   = &binexp32p4_table[*k1];
	}
	else {	bin_k   = *k0; }
	test0 = &binexp32p4_table[*k0];

	// binary
	nbytes_dec = rice_read_binary(
		&binary, src, nbytes_dec, cache, count, crc, bin_k
	);

	// value + state
	*value = (u32) ((((u32) unary) << bin_k) + binary);
	if PROBABLE ( test1 != NULL, 0.575 ){
		rice_update_dec(sum1, k1, *value, test1);
		*value += binexp32(*k0);
	}
	rice_update_dec(sum0, k0, *value, test0);

	return nbytes_dec;
}

//--------------------------------------------------------------------------//

/**@fn rice_read_unary
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
 *     8/16-bit :   34u
 *       24-bit : 4098u
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
ALWAYS_INLINE size_t
rice_read_unary(
	/*@out@*/ rice24 *const restrict unary, const u8 *const restrict src,
	size_t nbytes_dec, cache32 *const restrict cache,
	bitcnt *const restrict count, u32 *const restrict crc,
	const rice24 lax_limit
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
	*unary = (rice24) nbit;
	if IMPROBABLE ( nbit == *count, 0.25 ){
		do {	inbyte  = rice_crc32(src[nbytes_dec++], crc);
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

/**@fn rice_read_binary
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
rice_read_binary(
	/*@out@*/ rice24 *const restrict binary, const u8 *const restrict src,
	size_t nbytes_dec, cache32 *const restrict cache,
	bitcnt *const restrict count, u32 *const restrict crc,
	const bitcnt bin_k
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
		inbyte  = rice_crc32(src[nbytes_dec++], crc);
		*cache |= ((cache32) inbyte) << *count;
		*count += 8u;
	}
	*binary  = (rice24) (*cache & lsmask32(bin_k, SMM_DEC));
	*cache >>= bin_k;
	*count  -= bin_k;

	assert(*count <= (bitcnt) 8u);
	return nbytes_dec;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
