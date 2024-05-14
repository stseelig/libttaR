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
extern const u32 shift32p4_bit_table[];
/*@unchecked@*/ /*@unused@*/
extern const u32 lsmask32_table[];

//////////////////////////////////////////////////////////////////////////////

#undef rice
INLINE void
rice_init(
	/*@out@*/ register struct Rice *const restrict rice, register u8,
	register u8
)
/*@modifies	*rice@*/
;

//--------------------------------------------------------------------------//

ALWAYS_INLINE u32 shift32_bit(register u8) /*@*/;
ALWAYS_INLINE u32 shift32p4_bit(register u8, const enum ShiftMaskMode) /*@*/;
ALWAYS_INLINE u32 lsmask32(register u8, const enum ShiftMaskMode) /*@*/;

//--------------------------------------------------------------------------//

#undef sum
#undef k
/*@unused@*/
void rice_cmpsum(
	register u32 *const restrict sum, register u8 *const restrict k,
	register u32
)
/*@modifies	*sum,
		*k
@*/
;

#undef crc
ALWAYS_INLINE u8 rice_crc32(register u8, register u32 *const restrict crc)
/*@modifies	*crc@*/
;

//--------------------------------------------------------------------------//

#undef dest
#undef rice
#undef bitcache
#undef crc
ALWAYS_INLINE size_t
rice_encode(
	register u8 *const restrict dest, register size_t, register u32,
	register struct Rice *const restrict rice,
	register struct BitCache *const restrict bitcache,
	register u32 *const restrict crc
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
ALWAYS_INLINE size_t
rice_encode_cacheflush(
	register u8 *const restrict dest, register size_t,
	register u32 *const restrict cache, register u8 *const restrict count,
	register u32 *const restrict crc
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
ALWAYS_INLINE size_t
rice_unary_put(
	register u8 *const restrict dest, register size_t, register u32,
	register u32 *const restrict cache, register u8 *const restrict count,
	register u32 *const restrict crc
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
ALWAYS_INLINE size_t
rice_binary_put(
	register u8 *const restrict dest, register size_t, register u32,
	register u8, register u32 *const restrict cache,
	register u8 *const restrict count, register u32 *const restrict crc
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
	/*@out@*/ register u32 *const restrict value,
	register const u8 *const restrict, register size_t,
	register struct Rice *const restrict rice,
	register struct BitCache *const restrict bitcache,
	register u32 *const restrict crc
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
ALWAYS_INLINE size_t rice_unary_get(
	/*@out@*/ register u32 *const restrict unary,
	register const u8 *const restrict, register size_t,
	register u32 *const restrict cache, register u8 *const restrict count,
	register u32 *const restrict crc
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
ALWAYS_INLINE size_t rice_binary_get(
	/*@out@*/ register u32 *const restrict binary,
	register const u8 *const restrict, register size_t, register u8,
	register u32 *const restrict cache, register u8 *const restrict count,
	register u32 *const restrict crc
)
/*@modifies	*binary,
		*cache,
		*count,
		*crc
@*/
;

//////////////////////////////////////////////////////////////////////////////

INLINE void
rice_init(
	/*@out@*/ register struct Rice *const restrict rice, register u8 k0,
	register u8 k1
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

ALWAYS_INLINE u32
shift32_bit(register u8 k)
/*@*/
{
	return (u32) (0x1u << k);
}

// keeps rice.kN in check
ALWAYS_INLINE u32
shift32p4_bit(register u8 k, const enum ShiftMaskMode mode)
/*@*/
{
	register u32 r;

	switch ( mode ){
	case SMM_CONST:
	case SMM_SHIFT:
		r = (u32) (k != 0
			? 0x1u << ((u8) (k + 4u) <= (u8) 31u
				? (u8) (k + 4u) : (u8) 31u
			)
			: 0
		);
		break;
	case SMM_TABLE:
		r = shift32p4_bit_table[k];
		break;
	}
	return r;
}

ALWAYS_INLINE u32
lsmask32(register u8 k, const enum ShiftMaskMode mode)
/*@*/
{
	register u32 r;

	switch ( mode ){
	case SMM_CONST:
	case SMM_SHIFT:
		r = (u32) (k != 0 ? 0xFFFFFFFFu >> (32u - k) : 0 );
		break;
	case SMM_TABLE:
		r = lsmask32_table[k];
		break;
	}
	return r;
}

//==========================================================================//

// this procedure got macro'd because the compiler wasn't ALWAYS_INLINE'ing it
#define rice_cmpsum(sum, k, value) \
{ \
	*(sum) += (value) - (*(sum) >> 4u); \
	if ( *(sum) < shift32p4_bit(*(k), SMM_TABLE) ){ \
		--(*(k)); \
	} \
	else if ( *(sum) > shift32p4_bit(*(k) + 1u, SMM_TABLE) ){ \
		++(*(k)); \
	} else{;} \
}

// returns x
ALWAYS_INLINE u8
rice_crc32(register u8 x, register u32 *const restrict crc)
/*@modifies	*crc@*/
{
	*crc = crc32_cont(x, *crc);
	return x;
}

//==========================================================================//

// returns nbytes written to dest + r
ALWAYS_INLINE size_t
rice_encode(
	register u8 *const restrict dest, register size_t r,
	register u32 value, register struct Rice *const restrict rice,
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
		kx = *k1;
		rice_cmpsum(sum1, k1, value);
		unary = (value >> kx) + 1u;
	}

	r = rice_unary_put(dest, r, unary, cache, count, crc);
	if LIKELY ( kx != 0 ){
		binary = value & lsmask32(kx, SMM_SHIFT);
		r = rice_binary_put(dest, r, binary, kx, cache, count, crc);
	}

	return r;
}

// returns nbytes written to dest + r
ALWAYS_INLINE size_t
rice_encode_cacheflush(
	register u8 *const restrict dest, register size_t r,
	register u32 *const restrict cache, register u8 *const restrict count,
	register u32 *const restrict crc
)
/*@modifies	*dest,
		*cache,
		*count,
		*crc
@*/
{
	while ( *count != 0 ){
		dest[r++] = rice_crc32((u8) (*cache & 0xFFu), crc);
		*cache  >>= 8u;
		*count    = (*count > (u8) 8u ? *count - 8u : 0);
	}
	return r;
}

//--------------------------------------------------------------------------//

// returns nbytes written to dest + r
ALWAYS_INLINE size_t
rice_unary_put(
	register u8 *const restrict dest, register size_t r,
	register u32 unary, register u32 *const restrict cache,
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
			dest[r++] = rice_crc32((u8) (*cache & 0xFFu), crc);
			*cache  >>= 8u;
			*count   -= 8u;
		}
	} while UNLIKELY ( unary > (u32) 23u );

	*cache |= lsmask32((u8) unary, SMM_SHIFT) << *count;
	*count += unary + 1u;
	return r;
}

// returns nbytes written to dest + r
ALWAYS_INLINE size_t
rice_binary_put(
	register u8 *const restrict dest, register size_t r,
	register u32 binary, register u8 k,
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
		dest[r++] = rice_crc32((u8) (*cache & 0xFFu), crc);
		*cache  >>= 8u;
		*count   -= 8u;
	}
	*cache |= (binary & lsmask32(k, SMM_SHIFT)) << *count;
	*count += k;
	return r;
}

//==========================================================================//

// returns nbytes read from src + r
ALWAYS_INLINE size_t
rice_decode(
	/*@out@*/ register u32 *const restrict value,
	register const u8 *const restrict src, register size_t r,
	register struct Rice *const restrict rice,
	register struct BitCache *const restrict bitcache,
	register u32 *const restrict crc
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

	u32 unary, binary;
	register  u8 kx;
	register bool depth1;

	r = rice_unary_get(&unary, src, r, cache, count, crc);
	if LIKELY_P ( unary + 1u != 0, 0.575 ){
		kx = *k1;
		depth1 = true;
	}
	else {	unary = 0;
		kx = *k0;
		depth1 = false;
	}

	*value = unary;
	if LIKELY ( kx != 0 ){
		r = rice_binary_get(&binary, src, r, kx, cache, count, crc);
		*value = (*value << kx) + binary;
	}

	if LIKELY_P ( depth1, 0.575 ){
		rice_cmpsum(sum1, k1, *value);
		*value += shift32_bit(*k0);
	}
	rice_cmpsum(sum0, k0, *value);

	return r;
}

//--------------------------------------------------------------------------//

// returns nbytes read from src + r
ALWAYS_INLINE size_t
rice_unary_get(
	/*@out@*/ register u32 *const restrict unary,
	register const u8 *const restrict src, register size_t r,
	register u32 *const restrict cache, register u8 *const restrict count,
	register u32 *const restrict crc
)
/*@modifies	*unary,
		*cache,
		*count,
		*crc
@*/
{
	register union { u8 u_8; } t;

	// switched initial value from 0, because in rice_decode, depth1 is
	//  much more likely than not; moved a '--unary' in the depth1 branch
	//  to a 'unary = 0' in !depth1 branch
	*unary = UINT32_MAX;

#ifndef LIBTTAr_NO_INSTRUCTION_TZCNT
	// this loop is slightly better than the lookup-table one, as long as
	//  tbcnt32 maps to an instruction (tzcnt/ctz). otherwise a bit slower
	goto loop_entr;
	do {	*cache  = rice_crc32(src[r++], crc);
		*count  = (u8) 8u;
loop_entr:
		t.u_8 = (u8) tbcnt32(*cache);
		*unary += t.u_8;
	} while UNLIKELY_P ( t.u_8 == *count, 0.25 );
#else
	while UNLIKELY_P (
		(*cache ^ lsmask32(*count, SMM_TABLE)) == 0, 0.25
	){
		*unary += *count;
		*cache  = rice_crc32(src[r++], crc);
		*count  = (u8) 8u;
	}
	t.u_8 = (u8) tbcnt32(*cache);	// *cache is always < UINT8_MAX
	*unary  += t.u_8;
#endif
	*cache >>= t.u_8 + 1u;		// t.u_8 is always < 8
	*count  -= t.u_8 + 1u;
	return r;
}

// returns nbytes read from src + r
ALWAYS_INLINE size_t
rice_binary_get(
	/*@out@*/ register u32 *const restrict binary,
	register const u8 *const restrict src, register size_t r,
	register u8 k, register u32 *const restrict cache,
	register u8 *const restrict count, register u32 *const restrict crc
)
/*@modifies	*binary,
		*cache,
		*count,
		*crc
@*/
{
	while LIKELY_P ( *count < k, 0.9 ){
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
