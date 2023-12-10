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
static inline void pcm_write_u8(
	register u8 *const dest, register const i32 *const, register size_t
)
/*@modifies	*dest@*/
;

#undef dest
static inline void pcm_write_i16le(
	register u8 *const dest, register const i32 *const, register size_t
)
/*@modifies	*dest@*/
;

#undef dest
static inline void pcm_write_i24le(
	register u8 *const dest, register const i32 *const, register size_t
)
/*@modifies	*dest@*/
;

//--------------------------------------------------------------------------//

static inline u8 i32h_to_u8(register i32) /*@*/;

#undef dest
static inline void write_i32h_to_i16le(register u8 *const dest, register i32)
/*@modifies	*dest*/
;

#undef dest
static inline void write_i32h_to_i24le(register u8 *const dest, register i32)
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
	if ( (samplebytes == 0) || ( samplebytes > TTA_SAMPLEBYTES_MAX) ){
		return 0;
	}

	switch ( samplebytes ){
	case 1u:
		pcm_write_u8(dest, src, nsamples);
		break;
	case 2u:
		pcm_write_i16le(dest, src, nsamples);
		break;
	case 3u:
		pcm_write_i24le(dest, src, nsamples);
		break;
	}

	return nsamples;
}

//--------------------------------------------------------------------------//

static inline void
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
	return;
}

static inline void
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
	return;
}

static inline void
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
	return;
}

//--------------------------------------------------------------------------//

static inline u8
i32h_to_u8(register i32 x)
/*@*/
{
	return (u8) (x + 0x80u);
}

static inline void
write_i32h_to_i16le(register u8 *const dest, register i32 x)
/*@modifies	*dest*/
{
	register const u32 y = (u32) x;
	dest[0] = (u8)  y;
	dest[1] = (u8) (y >> 8u);
	return;
}

static inline void
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
