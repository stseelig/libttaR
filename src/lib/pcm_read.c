//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_read.c                                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// only u8, i16le, and i24le supported                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "../bits.h"

#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

#undef dest
ALWAYS_INLINE size_t pcm_read_u8(
	register i32 *const dest, register const u8 *const, register size_t
)
/*@modifies	*dest@*/
;

#undef dest
ALWAYS_INLINE size_t pcm_read_i16le(
	register i32 *const dest, register const u8 *const, register size_t
)
/*@modifies	*dest@*/
;

#undef dest
ALWAYS_INLINE size_t pcm_read_i24le(
	register i32 *const dest, register const u8 *const, register size_t
)
/*@modifies	*dest@*/
;

//--------------------------------------------------------------------------//

ALWAYS_INLINE i32 u8_to_i32h(register u8) /*@*/;

#undef dest
ALWAYS_INLINE void read_i16le_to_i32h(
	register i32 *const dest, register const u8 *const
)
/*@modifies	*dest@*/
;

#undef dest
ALWAYS_INLINE void read_i24le_to_i32h(
	register i32 *const dest, register const u8 *const
)
/*@modifies	*dest@*/
;

//////////////////////////////////////////////////////////////////////////////

// returns nsamples on success
size_t
libttaR_pcm_read(
	i32 *const dest, const u8 *const src, size_t nsamples,
	enum TTASampleBytes samplebytes
)
/*@modifies	*dest@*/
{
	size_t r = 0;

	switch ( samplebytes ){
	case 1u:
		r = pcm_read_u8(dest, src, nsamples);
		break;
	case 2u:
		r = pcm_read_i16le(dest, src, nsamples);
		break;
	case 3u:
		r = pcm_read_i24le(dest, src, nsamples);
		break;
	}
	return r;
}

//--------------------------------------------------------------------------//

// returns nsamples
ALWAYS_INLINE size_t
pcm_read_u8(
	register i32 *const dest, register const u8 *const src,
	register size_t nsamples
)
/*@modifies	*dest@*/
{
	register size_t i;
	for ( i = 0; i < nsamples; ++i ){
		dest[i] = u8_to_i32h(src[i]);
	}
	return i;
}

// returns nsamples
ALWAYS_INLINE size_t
pcm_read_i16le(
	register i32 *const dest, register const u8 *const src,
	register size_t nsamples
)
/*@modifies	*dest@*/
{
	register size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += 2u ){
		read_i16le_to_i32h(&dest[i], &src[j]);
	}
	return i;
}

// returns nsamples
ALWAYS_INLINE size_t
pcm_read_i24le(
	register i32 *const dest, register const u8 *const src,
	register size_t nsamples
)
/*@modifies	*dest@*/
{
	register size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += 3u ){
		read_i24le_to_i32h(&dest[i], &src[j]);
	}
	return i;
}

//--------------------------------------------------------------------------//

ALWAYS_INLINE i32
u8_to_i32h(register u8 x)
/*@*/
{
	return (i32) (x - 0x80u);
}

ALWAYS_INLINE void
read_i16le_to_i32h(register i32 *const dest, register const u8 *const src)
/*@modifies	*dest@*/
{
	register u32 *const t = (u32 *) dest;
	*t  =  (u32)       src[0];
	*t |= ((u32) ((i8) src[1])) << 8u;
	return;
}

ALWAYS_INLINE void
read_i24le_to_i32h(register i32 *const dest, register const u8 *const src)
/*@modifies	*dest@*/
{
	register u32 *const t = (u32 *) dest;
	*t  =  (u32)       src[0];
	*t |= ((u32)       src[1])  <<  8u;
	*t |= ((u32) ((i8) src[2])) << 16u;
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
