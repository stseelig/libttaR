//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/pcm_write.c                                                        //
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
ALWAYS_INLINE size_t pcm_write_u8(
	register u8 *const dest, register const i32 *const, register size_t
)
/*@modifies	*dest@*/
;

#undef dest
ALWAYS_INLINE size_t pcm_write_i16le(
	register u8 *const dest, register const i32 *const, register size_t
)
/*@modifies	*dest@*/
;

#undef dest
ALWAYS_INLINE size_t pcm_write_i24le(
	register u8 *const dest, register const i32 *const, register size_t
)
/*@modifies	*dest@*/
;

//--------------------------------------------------------------------------//

ALWAYS_INLINE u8 i32h_to_u8(register i32) /*@*/;

#undef dest
ALWAYS_INLINE void write_i32h_to_i16le(register u8 *const dest, register i32)
/*@modifies	*dest*/
;

#undef dest
ALWAYS_INLINE void write_i32h_to_i24le(register u8 *const dest, register i32)
/*@modifies	*dest*/
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
	size_t r = 0;

	switch ( samplebytes ){
	case 1u:
		r = pcm_write_u8(dest, src, nsamples);
		break;
	case 2u:
		r = pcm_write_i16le(dest, src, nsamples);
		break;
	case 3u:
		r = pcm_write_i24le(dest, src, nsamples);
		break;
	}
	return r;
}

//--------------------------------------------------------------------------//

// returns nsamples
ALWAYS_INLINE size_t
pcm_write_u8(
	register u8 *const dest, register const i32 *const src,
	register size_t nsamples
)
/*@modifies	*dest@*/
{
	register size_t i;
	for ( i = 0; i < nsamples; ++i ){
		dest[i] = i32h_to_u8(src[i]);
	}
	return i;
}

// returns nsamples
ALWAYS_INLINE size_t
pcm_write_i16le(
	register u8 *const dest, register const i32 *const src,
	register size_t nsamples
)
/*@modifies	*dest@*/
{
	register size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += 2u ){
		write_i32h_to_i16le(&dest[j], src[i]);
	}
	return i;
}

// returns nsamples
ALWAYS_INLINE size_t
pcm_write_i24le(
	register u8 *const dest, register const i32 *const src,
	register size_t nsamples
)
/*@modifies	*dest@*/
{
	register size_t i, j;
	for ( i = 0, j = 0; i < nsamples; ++i, j += 3u ){
		write_i32h_to_i24le(&dest[j], src[i]);
	}
	return i;
}

//--------------------------------------------------------------------------//

ALWAYS_INLINE u8
i32h_to_u8(register i32 x)
/*@*/
{
	return (u8) (x + 0x80u);
}

ALWAYS_INLINE void
write_i32h_to_i16le(register u8 *const dest, register i32 x)
/*@modifies	*dest*/
{
	register const u32 y = (u32) x;
	dest[0] = (u8)  y;
	dest[1] = (u8) (y >> 8u);
	return;
}

ALWAYS_INLINE void
write_i32h_to_i24le(register u8 *const dest, register i32 x)
/*@modifies	*dest*/
{
	register const u32 y = (u32) x;
	dest[0] = (u8)  y;
	dest[1] = (u8) (y >>  8u);
	dest[2] = (u8) (y >> 16u);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
