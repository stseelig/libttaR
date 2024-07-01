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

#include <stdbool.h>
#include <stddef.h>	// size_t

#include "../bits.h"
#include "../splint.h"

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

//--------------------------------------------------------------------------//

// max unary size:
//	8/16-bit :   16u bytes + 7u bits + terminator
//	  24-bit : 4096u bytes + 7u bits + terminator
// the lax_limit has an extra byte to make handling invalid data faster/easier
#define UNARY_LAX_LIMIT_1_2		((u32) ((8u *   18u) - 1u))
#define UNARY_LAX_LIMIT_3		((u32) ((8u * 4098u) - 1u))

//////////////////////////////////////////////////////////////////////////////

struct Rice {
	u32	sum[2u];
	u8	k[2u];
};

struct BitCache {
	u32	cache;
	u8	count;
};

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 shift32p4_bit_table[];
/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 lsmask32_table[];

//////////////////////////////////////////////////////////////////////////////

#undef rice
INLINE void rice_init(/*@out@*/ struct Rice *restrict rice, u8, u8)
/*@modifies	*rice@*/
;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST u32 shift32_bit(u8) /*@*/;
ALWAYS_INLINE CONST u32 shift32p4_bit(u8, enum ShiftMaskMode) /*@*/;
ALWAYS_INLINE CONST u32 lsmask32(u8, enum ShiftMaskMode) /*@*/;

//--------------------------------------------------------------------------//

#undef sum
#undef k
ALWAYS_INLINE void rice_cmpsum(u32 *restrict sum, u8 *restrict k, u32)
/*@modifies	*sum,
		*k
@*/
;

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
ALWAYS_INLINE size_t rice_unary_write(
	/*@partial@*/ u8 *restrict dest, u32, size_t, u32 *restrict cache,
	u8 *restrict count, u32 *restrict crc
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
ALWAYS_INLINE size_t rice_binary_write(
	/*@partial@*/ u8 *restrict dest, u32, size_t, u8, u32 *restrict cache,
	u8 *restrict count, u32 *restrict crc
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
	u32 *restrict crc, u32
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
ALWAYS_INLINE size_t rice_unary_read(
	/*@out@*/ u32 *restrict unary, const u8 *restrict, size_t,
	u32 *restrict cache, u8 *restrict count, u32 *restrict crc, u32
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
ALWAYS_INLINE size_t rice_binary_read(
	/*@out@*/ u32 *restrict binary, const u8 *restrict, size_t, u8,
	u32 *restrict cache, u8 *restrict count, u32 *restrict crc
)
/*@modifies	*binary,
		*cache,
		*count,
		*crc
@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn rice_init
 * @brief initializes a 'struct Rice'
 *
 * @param rice[out] struct to initialize
 * @param k0 initial value for rice->k[0u]
 * @param k1 initial value for rice->k[1u]
**/
INLINE void
rice_init(
	/*@out@*/ register struct Rice *const restrict rice,
	register const u8 k0, register const u8 k1
)
/*@modifies	*rice@*/
{
	rice->sum[0u] = shift32p4_bit(k0, SMM_CONST);
	rice->sum[1u] = shift32p4_bit(k1, SMM_CONST);
	rice->k[0u]   = k0;
	rice->k[1u]   = k1;
	return;
}

//==========================================================================//

/**@fn shift32_bit
 * @brief shift the 0th bit 32-bit 'k' places left
 *
 * @param k bit number
 *
 * @return a 32-bit mask with only the 'k'th bit set
 *
 * @pre 0 <= 'k' <= 31u
**/
ALWAYS_INLINE CONST u32
shift32_bit(register const u8 k)
/*@*/
{
	return (u32) (0x1u << k);
}

/**@fn shift32p4_bit
 * @brief shift the 0th bit 32-bit 'k' + 4u places left
 *
 * @param k bit number - 4u
 * @param mode constant, shift, or lookup table
 *
 * @return a mask with only the ('k' + 4u)th bit set, 0, or 0xFFFFFFFFu
 *
 * @pre 0 <= 'k' <= 28u
**/
ALWAYS_INLINE CONST u32
shift32p4_bit(register const u8 k, const enum ShiftMaskMode mode)
/*@*/
{
	register u32 r;
	switch ( mode ){
	case SMM_CONST:
	case SMM_SHIFT:
		r = (u32) (k != 0
			? (k < (u8) 28u ? 0x10u << k : 0xFFFFFFFFu) : 0
		);
		break;
	case SMM_TABLE:
		r = shift32p4_bit_table[k];
		break;
	}
	return r;
}

/**@fn lsmask32
 * @brief least significant mask 32-bit
 *
 * @param k number of bits in the mask
 * @param mode constant, shift, or lookup table
 *
 * @return a mask with 'k' low bits set
 *
 * @pre 0 <= 'k' <= 27u
**/
ALWAYS_INLINE CONST u32
lsmask32(register const u8 k, const enum ShiftMaskMode mode)
/*@*/
{
	register u32 r;
	switch ( mode ){
	case SMM_CONST:
	case SMM_SHIFT:
		r = (u32) ((0x1u << k) - 1u);
		break;
	case SMM_TABLE:
		r = lsmask32_table[k];
		break;
	}
	return r;
}

//==========================================================================//

/**@fn rice_cmpsum
 * @brief update the rice struct data
 *
 * @param sum[in out] rice->sum[]
 * @param k[in out] rice->k[]
 * @param value input value to code
 *
 * @pre  0 <= 'k' <= 27u
 * @post 0 <= 'k' <= 27u
 *
 * @note in practice, 'k' maxes out at 24u
**/
ALWAYS_INLINE void
rice_cmpsum(
	register u32 *const restrict sum, register u8 *const restrict k,
	register const u32 value
)
/*@modifies	*sum,
		*k
@*/
{
	*sum += value - (*sum >> 4u);
	if UNLIKELY ( *sum < shift32p4_bit(*k, SMM_TABLE) ){
		*k -= 1u;
	}
	else if UNLIKELY ( *sum > shift32p4_bit(*k + 1u, SMM_TABLE) ){
		*k += 1u;
	} else{;}
	return;
}

/**@fn rice_crc32
 * @brief add a byte to a CRC
 *
 * @param x input byte
 * @param crc[in out] current CRC
 *
 * @return 'x'
**/
ALWAYS_INLINE u8
rice_crc32(register const u8 x, register u32 *const restrict crc)
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
 * @param r index of 'dest'
 * @param value value to encode
 * @param rice[in out] the rice code data for the current channel
 * @param bitcache[in out] the bitcache data
 * @param crc[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'r'
 *
 * @note max write size (unary + binary):
 *     8/16-bit :   19u
 *       24-bit : 4099u
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
ALWAYS_INLINE size_t
rice_encode(
	/*@partial@*/ register u8 *const restrict dest, register u32 value,
	register size_t r, register struct Rice *const restrict rice,
	register struct BitCache *const restrict bitcache,
	register u32 *const restrict crc
)
/*@modifies	*dest,
		*rice,
		*bitcache,
		*crc
@*/
{
	register u32 *const restrict sum0  = &rice->sum[0u];
	register u32 *const restrict sum1  = &rice->sum[1u];
	register  u8 *const restrict k0    = &rice->k[0u];
	register  u8 *const restrict k1    = &rice->k[1u];
	register u32 *const restrict cache = &bitcache->cache;
	register  u8 *const restrict count = &bitcache->count;

	register u32 unary = 0, binary;
	register  u8 kx;
	register u32 depth1_trigger;

	kx = *k0;
	rice_cmpsum(sum0, k0, value);

	depth1_trigger = shift32_bit(kx);
	if LIKELY_P ( value >= depth1_trigger, 0.575 ){
		value -= depth1_trigger;
		kx     = *k1;
		rice_cmpsum(sum1, k1, value);
		unary  = (value >> kx) + 1u;
	}

	r = rice_unary_write(dest, unary, r, cache, count, crc);
	if LIKELY ( kx != 0 ){
		binary = value & lsmask32(kx, SMM_ENC);
		r = rice_binary_write(dest, binary, r, kx, cache, count, crc);
	}

	return r;
}

/**@fn rice_encode_cacheflush
 * @brief flush any data left in the 'bitcache' to 'dest'
 *
 * @param dest[out] destination buffer
 * @param r index of 'dest'
 * @param bitcache[in out] the bitcache data
 * @param crc[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'r'
 *
 * @note max write size: 4u
**/
INLINE size_t
rice_encode_cacheflush(
	/*@partial@*/ register u8 *const restrict dest, register size_t r,
	register struct BitCache *const restrict bitcache,
	register u32 *const restrict crc
)
/*@modifies	*dest,
		*bitcache,
		*crc
@*/
{
	register u32 *const restrict cache = &bitcache->cache;
	register  u8 *const restrict count = &bitcache->count;

	while ( *count != 0 ){
		dest[r++] = rice_crc32((u8) *cache, crc);
		*cache  >>= 8u;
		*count    = (*count > (u8) 8u ? *count - 8u : 0);
	}
	return r;
}

//--------------------------------------------------------------------------//

/**@fn rice_unary_write
 * @brief write a unary code to 'dest'
 *
 * @param dest[out] destination buffer
 * @param r index of 'dest'
 * @param unary the unary code
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'r'
 *
 * @note max write size:
 *	 8/16-bit :   16u
 *	   24-bit : 4096u
 * @note affected by LIBTTAr_OPT_PREFER_LOOKUP_TABLES
**/
ALWAYS_INLINE size_t
rice_unary_write(
	/*@partial@*/ register u8 *const restrict dest, register u32 unary,
	register size_t r, register u32 *const restrict cache,
	register u8 *const restrict count, register u32 *const restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	goto loop_entr;
	do {	unary  -= 23u;
		*cache |= lsmask32((u8) 23u, SMM_CONST) << *count;
		*count += 23u;
loop_entr:
		while ( *count >= (u8) 8u ){
			dest[r++] = rice_crc32((u8) *cache, crc);
			*cache  >>= 8u;
			*count   -= 8u;
		}
	} while UNLIKELY ( unary > (u32) 23u );

	*cache |= lsmask32((u8) unary, SMM_ENC) << *count;
	*count += unary + 1u;
	return r;
}

/**@fn rice_binary_write
 * @brief write a binary code to 'dest'
 *
 * @param dest[out] destination buffer
 * @param r index of 'dest'
 * @param binary the binary code
 * @param k from rice->k[], kx
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 *
 * @return number of bytes written to 'dest' + 'r'
 *
 * @note max write size: 3u
**/
ALWAYS_INLINE size_t
rice_binary_write(
	/*@partial@*/ register u8 *const restrict dest,
	register const u32 binary, register size_t r, register const u8 k,
	register u32 *const restrict cache, register u8 *const restrict count,
	register u32 *const restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	while ( *count >= (u8) 8u ){
		dest[r++] = rice_crc32((u8) *cache, crc);
		*cache  >>= 8u;
		*count   -= 8u;
	}
	*cache |= binary << *count;
	*count += k;
	return r;
}

//==========================================================================//

/**@fn rice_decode
 * @brief decode rice codes into a filtered i32 value
 *
 * @param value[in out] value to decode
 * @param src[in] source buffer
 * @param r index of 'src'
 * @param rice[in out] the rice code data for the current channel
 * @param bitcache[in out] the bitcache data
 * @param crc[in out] the current CRC
 * @param unary_lax_limit limit for the unary code
 *
 * @return number of bytes read from 'src' + 'r'
 *
 * @note max read size (unary + binary):
 *     8/16-bit :   22u
 *       24-bit : 4102u
 * @see rice_binary_read
**/
ALWAYS_INLINE size_t
rice_decode(
	/*@out@*/ register u32 *const restrict value,
	register const u8 *const restrict src, register size_t r,
	register struct Rice *const restrict rice,
	register struct BitCache *const restrict bitcache,
	register u32 *const restrict crc, register const u32 unary_lax_limit
)
/*@modifies	*value,
		*rice,
		*bitcache,
		*crc
@*/
{
	register u32 *const restrict sum0  = &rice->sum[0u];
	register u32 *const restrict sum1  = &rice->sum[1u];
	register  u8 *const restrict k0    = &rice->k[0u];
	register  u8 *const restrict k1    = &rice->k[1u];
	register u32 *const restrict cache = &bitcache->cache;
	register  u8 *const restrict count = &bitcache->count;

	u32 unary, binary = 0;
	register  u8 kx;
	register bool depth1;

	r = rice_unary_read(
		&unary, src, r, cache, count, crc, unary_lax_limit
	);
	if LIKELY_P ( unary != 0, 0.575 ){
		unary -= 1u;
		kx     = *k1;
		depth1 = true;
	}
	else {	kx     = *k0;
		depth1 = false;
	}

	if LIKELY ( kx != 0 ){
		r = rice_binary_read(&binary, src, r, kx, cache, count, crc);
	}
	*value = (unary << kx) + binary;

	if LIKELY_P ( depth1, 0.575 ){
		rice_cmpsum(sum1, k1, *value);
		*value += shift32_bit(*k0);
	}
	rice_cmpsum(sum0, k0, *value);

	return r;
}

//--------------------------------------------------------------------------//

/**@fn rice_unary_read
 * @brief read a unary code from 'src'
 *
 * @param unary[out] the unary code
 * @param src[in] source buffer
 * @param r index of 'src'
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 * @param lax_limit limit for the unary code. has an extra byte of margin, so
 *   if it is surpased, then the data is definitely invalid (corrupt or
 *   malicious). this would be caused by an overly long string of 0xFFu bytes
 *   in the source
 *
 * @return number of bytes read from 'src' + 'r'
 *
 * @note max read size:
 *     8/16-bit :   18u
 *       24-bit : 4098u
**/
ALWAYS_INLINE size_t
rice_unary_read(
	/*@out@*/ register u32 *const restrict unary,
	register const u8 *const restrict src, register size_t r,
	register u32 *const restrict cache, register u8 *const restrict count,
	register u32 *const restrict crc, register const u32 lax_limit
)
/*@modifies	*unary,
		*cache,
		*count,
		*crc
@*/
{
	register u8 nbits;

	nbits  = (u8) tbcnt32(*cache);
	*unary = nbits;
	if UNLIKELY_P ( nbits == *count, 0.25 ){
		do {	*cache  = rice_crc32(src[r++], crc);
			nbits   = (u8) tbcnt32(*cache);
			*unary += nbits;
			if UNLIKELY ( *unary > lax_limit ){
				nbits = 0;	// prevents *count underflow
				break;
			}
		} while UNLIKELY ( nbits == (u8) 8u );
		*count = (u8) 8u;
	}
	*cache >>= nbits + 1u;
	*count  -= nbits + 1u;
	return r;
}

/**@fn rice_binary_read
 * @brief read a binary code from 'src'
 *
 * @param binary[out] the binary code
 * @param src[in] source buffer
 * @param r index of 'src'
 * @param k from rice->k[], kx
 * @param cache[in out] the bitcache
 * @param count[in out] number of active bits in the 'cache'
 * @param crc[in out] the current CRC
 *
 * @return number of bytes read from 'src' + 'r'
 *
 * @note max read size: 4u (normally 3u, but might be 4u with malformed data)
**/
ALWAYS_INLINE size_t
rice_binary_read(
	/*@out@*/ register u32 *const restrict binary,
	register const u8 *const restrict src, register size_t r,
	register const u8 k, register u32 *const restrict cache,
	register u8 *const restrict count, register u32 *const restrict crc
)
/*@modifies	*binary,
		*cache,
		*count,
		*crc
@*/
{
	while LIKELY_P ( *count < k, 0.9 ){
		// a check like in the unary reader is unnecessary
		*cache |= rice_crc32(src[r++], crc) << *count;
		*count += 8u;
	}
	*binary = *cache & lsmask32(k, SMM_DEC);
	*cache  = (*cache >> k) & lsmask32(*count - k, SMM_DEC);
	*count -= k;
	return r;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
