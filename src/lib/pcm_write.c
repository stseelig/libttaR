//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_write.c                                                        //
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
NOINLINE size_t pcm_write_u8(u8 *const dest, const i32 *const, size_t)
/*@modifies	*dest@*/
;

#undef dest
NOINLINE size_t pcm_write_i16le(u8 *const dest, const i32 *const, size_t)
/*@modifies	*dest@*/
;

#undef dest
NOINLINE size_t pcm_write_i24le(u8 *const dest, const i32 *const, size_t)
/*@modifies	*dest@*/
;

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST u8 i32h_to_u8(register i32) /*@*/;

#undef dest
ALWAYS_INLINE void write_i32h_to_i16le(register u8 *const dest, register i32)
/*@modifies	*dest@*/
;

#undef dest
ALWAYS_INLINE void write_i32h_to_i24le(register u8 *const dest, register i32)
/*@modifies	*dest@*/
;

//////////////////////////////////////////////////////////////////////////////

// returns nsamples on success
size_t
libttaR_pcm_write(
	u8 *const dest, const i32 *const src, size_t nsamples,
	enum TTASampleBytes samplebytes
)
/*@modifies	*dest@*/
{
	switch ( samplebytes ){
	case TTASAMPLEBYTES_1:
		return pcm_write_u8(dest, src, nsamples);
	case TTASAMPLEBYTES_2:
		return pcm_write_i16le(dest, src, nsamples);
	case TTASAMPLEBYTES_3:
		return pcm_write_i24le(dest, src, nsamples);
	}
	return 0;
}

//--------------------------------------------------------------------------//

// returns nsamples
NOINLINE size_t
pcm_write_u8(u8 *const dest, const i32 *const src, size_t nsamples)
/*@modifies	*dest@*/
{
	size_t i;
	for ( i = 0; i < nsamples; ++i ){
		dest[i] = i32h_to_u8(src[i]);
	}
	return i;
}

// returns nsamples
NOINLINE size_t
pcm_write_i16le(u8 *const dest, const i32 *const src, size_t nsamples)
/*@modifies	*dest@*/
{
	size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += (size_t) 2u ){
		write_i32h_to_i16le(&dest[j], src[i]);
	}
	return i;
}

// returns nsamples
NOINLINE size_t
pcm_write_i24le(u8 *const dest, const i32 *const src, size_t nsamples)
/*@modifies	*dest@*/
{
	size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += (size_t) 3u ){
		write_i32h_to_i24le(&dest[j], src[i]);
	}
	return i;
}

//--------------------------------------------------------------------------//

ALWAYS_INLINE CONST u8
i32h_to_u8(register i32 x)
/*@*/
{
	return (u8) (x + 0x80u);
}

ALWAYS_INLINE void
write_i32h_to_i16le(register u8 *const dest, register i32 x)
/*@modifies	*dest@*/
{
	dest[0u] = (u8)  ((u32) x);
	dest[1u] = (u8) (((u32) x) >> 8u);
	return;
}

ALWAYS_INLINE void
write_i32h_to_i24le(register u8 *const dest, register i32 x)
/*@modifies	*dest@*/
{
	dest[0u] = (u8)  ((u32) x);
	dest[1u] = (u8) (((u32) x) >>  8u);
	dest[2u] = (u8) (((u32) x) >> 16u);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
