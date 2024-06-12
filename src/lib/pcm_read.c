//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_read.c                                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
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
NOINLINE size_t pcm_read_u8(i32 *const dest, const u8 *const, size_t)
/*@modifies	*dest@*/
;

#undef dest
NOINLINE size_t pcm_read_i16le(i32 *const dest, const u8 *const, size_t)
/*@modifies	*dest@*/
;

#undef dest
NOINLINE size_t pcm_read_i24le(i32 *const dest, const u8 *const, size_t)
/*@modifies	*dest@*/
;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST i32 u8_to_i32h(register u8) /*@*/;

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
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
		return pcm_read_u8(dest, src, nsamples);
	case TTASAMPLEBYTES_2:
		return pcm_read_i16le(dest, src, nsamples);
	case TTASAMPLEBYTES_3:
		return pcm_read_i24le(dest, src, nsamples);
	}
	return 0;
}

//--------------------------------------------------------------------------//

// returns nsamples
NOINLINE size_t
pcm_read_u8(i32 *const dest, const u8 *const src, size_t nsamples)
/*@modifies	*dest@*/
{
	size_t i;
	for ( i = 0; i < nsamples; ++i ){
		dest[i] = u8_to_i32h(src[i]);
	}
	return i;
}

// returns nsamples
NOINLINE size_t
pcm_read_i16le(i32 *const dest, const u8 *const src, size_t nsamples)
/*@modifies	*dest@*/
{
	size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += (size_t) 2u ){
		read_i16le_to_i32h(&dest[i], &src[j]);
	}
	return i;
}

// returns nsamples
NOINLINE size_t
pcm_read_i24le(i32 *const dest, const u8 *const src, size_t nsamples)
/*@modifies	*dest@*/
{
	size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += (size_t) 3u ){
		read_i24le_to_i32h(&dest[i], &src[j]);
	}
	return i;
}

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST i32
u8_to_i32h(register u8 x)
/*@*/
{
	return (i32) (x - 0x80u);
}

ALWAYS_INLINE void
read_i16le_to_i32h(register i32 *const dest, register const u8 *const src)
/*@modifies	*dest@*/
{
	*((u32 *) dest)  =  (u32)       src[0u];
	*((u32 *) dest) |= ((u32) ((i8) src[1u])) << 8u;
	return;
}

ALWAYS_INLINE void
read_i24le_to_i32h(register i32 *const dest, register const u8 *const src)
/*@modifies	*dest@*/
{
	*((u32 *) dest)  =  (u32)       src[0u];
	*((u32 *) dest) |= ((u32)       src[1u])  <<  8u;
	*((u32 *) dest) |= ((u32) ((i8) src[2u])) << 16u;
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
