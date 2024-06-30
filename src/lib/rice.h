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

// max unary size:
//	8/16-bit :   16u bytes + 7u bits
//	  24-bit : 4096u bytes + 7u bits
#define UNARY_SOFT_LIMIT_1_2		((u32) (8u *   16u))
#define UNARY_SOFT_LIMIT_3		((u32) (8u * 4096u))
#define UNARY_HARD_LIMIT(soft_limit)	((u32) ((soft_limit) + 7u))

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
			? (k < (u8) 28u ? 0x10u << (u8) k : 0xFFFFFFFFu) : 0
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
 *
 * @note affected by LIBTTAr_OPT_CMOV_SHIFTER
**/
ALWAYS_INLINE CONST u32
lsmask32(register const u8 k, const enum ShiftMaskMode mode)
/*@*/
{
	register u32 r;
	switch ( mode ){
	case SMM_CONST:
	case SMM_SHIFT:
#ifndef LIBTTAr_OPT_CMOV_SHIFTER
		r = (u32) (0x1u << k) - 1u;
#else
		// can be faster on older x86
		r = (u32) (k != 0 ? 0xFFFFFFFFu >> ((u8) 32u - k) : 0);
#endif
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
	register union { u32 u_32; } t;

	kx = *k0;
	rice_cmpsum(sum0, k0, value);

	t.u_32 = shift32_bit(kx);
	if LIKELY_P ( value >= t.u_32, 0.575 ){
		value -= t.u_32;
		kx     = *k1;
		rice_cmpsum(sum1, k1, value);
		unary  = (value >> kx) + 1u;
	}

	r = rice_unary_write(dest, unary, r, cache, count, crc);
	if LIKELY ( kx != 0 ){
		binary = value & lsmask32(kx, SMM_SHIFT);
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

	*cache |= lsmask32((u8) unary, SMM_SHIFT) << *count;
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
 * @param unary_soft_limit limit for the unary code
 *
 * @return number of bytes read from 'src' + 'r'
 *
 * @note max read size (unary + binary):
 *     8/16-bit :   21u
 *       24-bit : 4101u
 * @see rice_binary_read
**/
ALWAYS_INLINE size_t
rice_decode(
	/*@out@*/ register u32 *const restrict value,
	register const u8 *const restrict src, register size_t r,
	register struct Rice *const restrict rice,
	register struct BitCache *const restrict bitcache,
	register u32 *const restrict crc, register const u32 unary_soft_limit
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
		&unary, src, r, cache, count, crc, unary_soft_limit
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
 * @param unary_soft_limit limit for the unary code
 *
 * @return number of bytes read from 'src' + 'r'
 *
 * @pre 0 <= '*count' <= 7u
 *
 * @note max read size:
 *     8/16-bit :   17u
 *       24-bit : 4097u
 * @note affected by LIBTTAr_OPT_NO_TZCNT
**/
ALWAYS_INLINE size_t
rice_unary_read(
	/*@out@*/ register u32 *const restrict unary,
	register const u8 *const restrict src, register size_t r,
	register u32 *const restrict cache, register u8 *const restrict count,
	register u32 *const restrict crc, register const u32 soft_limit
)
/*@modifies	*unary,
		*cache,
		*count,
		*crc
@*/
{
	register union { u8 u_8; } t;

	// reverted to 0, becasuse UINT32_MAX was ruining the safety check
	*unary = 0;

#ifndef LIBTTAr_OPT_NO_TZCNT
	// this loop is slightly better than the lookup-table one, as long as
	//   tbcnt32 is an instruction (tzcnt/ctz). otherwise a bit slower
	goto loop_entr;
	do {	*cache  = rice_crc32(src[r++], crc);
		*count  = (u8) 8u;
loop_entr:
		t.u_8 = (u8) tbcnt32(*cache);
		*unary += t.u_8;
		if UNLIKELY ( *unary > soft_limit ){ goto unary_check; }
	} while UNLIKELY_P ( t.u_8 == *count, 0.25 );
#else
	while UNLIKELY_P (
		(*cache ^ lsmask32(*count, SMM_TABLE)) == 0, 0.25
	){
		if UNLIKELY ( *unary > soft_limit - 8u ){ goto unary_check; }
		*unary += *count;
		*cache  = rice_crc32(src[r++], crc);
		*count  = (u8) 8u;
	}
	t.u_8 = (u8) tbcnt32(*cache);
	*unary  += t.u_8;
#endif
loop_end:
	// *cache should always be != 0xFFu
	//  t.u_8 should always be <= 7u
	*cache >>= t.u_8 + 1u;
	*count  -= t.u_8 + 1u;
	return r;

unary_check:
#ifdef LIBTTAr_OPT_NO_TZCNT
	t.u_8 = (u8) tbcnt32(*cache);
	*unary  += t.u_8;
#endif
	// the last byte read should not be 0xFFu (the unary code ends with a
	//   0-bit). if it is, *count will underflow, which will cause an
	//   out-of-bounds read of lsmask32_table in the binary decoder
	// when the data is malformed, having "correct" values for 't.u_8' and
	//   '*unary' should not matter, because the data is garbage anyway
	if ( t.u_8 == (u8) 8u ){
		if ( *unary <= UNARY_HARD_LIMIT(soft_limit) ){
			// could be valid
			*cache = rice_crc32(src[r++], crc);
			t.u_8  = 0;	// assuming not malformed
		}
		else {	// malformed
			t.u_8  = (u8) 7u;
		}
	}
	goto loop_end;
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
 * @note max read size: 4u
 * @note max read size normally is 3u, but might be 4u with malformed data
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
		// a check like in the unary decoder should be unnecessary
		*cache |= rice_crc32(src[r++], crc) << *count;
		*count += 8u;
	}
	*binary = *cache & lsmask32(k, SMM_TABLE);
	*cache  = (*cache >> k) & lsmask32(*count - k, SMM_TABLE);
	*count -= k;
	return r;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
