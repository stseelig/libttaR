#ifndef TTA_CODEC_COMMON_H
#define TTA_CODEC_COMMON_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/common.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "../bits.h"

//////////////////////////////////////////////////////////////////////////////

enum TTASampleBytes {
	TTASAMPLEBYTES_1 = 1u,
	TTASAMPLEBYTES_2 = 2u,
	TTASAMPLEBYTES_3 = 3u
};
#define TTA_SAMPLEBYTES_MAX	((uint) TTASAMPLEBYTES_3)
#define TTA_SAMPLEBITS_MAX	((uint) (8u*TTA_SAMPLEBYTES_MAX))

//////////////////////////////////////////////////////////////////////////////

struct BitCache {
	u32	cache;
	u8	count;
};

struct Rice {
	u32	sum[2u];
	u8	k[2u];
};

struct Filter {
	i32	error;
	i32	qm[8u];
	i32	dx[9u];	// the extra value is for a memmove trick
	i32	dl[9u];	// ~
};

struct Codec {
	struct Filter	filter;
	struct Rice	rice;
	i32		prev;
};

struct LibTTAr_CodecState_Priv {
	struct BitCache	bitcache;
	struct Codec 	codec[];
};

struct LibTTAr_CodecState_User {
	u32	ncalls_codec;
	u32	crc;
	size_t	ni32;			// enc: num read, dec: num written
	size_t	ni32_total;		// ~
	size_t	nbytes_tta;		// enc: num written, dec: num read
	size_t	nbytes_tta_total;	// ~
};

//////////////////////////////////////////////////////////////////////////////

/**@fn rice_init
 * @brief initializes a 'struct Rice'
 *
 * @param rice[out] struct to initialize
**/
INLINE void
rice_init(/*@out@*/ register struct Rice *const restrict rice)
/*@modifies	*rice@*/
{
	rice->sum[0u] = (u32) 0x00004000u;
	rice->sum[1u] = (u32) 0x00004000u;
	rice->k[0u]   = (u8) 10u;
	rice->k[1u]   = (u8) 10u;
	return;
}

/**@fn codec_init
 * @brief initializes an array of 'struct Codec'
 *
 * @param codec[out] the struct array to initialize
 * @param nchan number of audio channels
**/
INLINE void
codec_init(
	/*@out@*/ register struct Codec *const restrict codec,
	register const uint nchan
)
/*@modifies	*codec@*/
{
	register uint i;
	for ( i = 0; i < nchan; ++i ){
		MEMSET(&codec[i].filter, 0x00, sizeof codec[i].filter);
		rice_init(&codec[i].rice);
		codec[i].prev = 0;
	}
	return;
}

/**@fn state_priv_init
 * @brief initializes a private state struct
 *
 * @param priv[out] the private state struct
 * @param nchan number of audio channels
**/
INLINE void
state_priv_init(
	/*@out@*/
	register struct LibTTAr_CodecState_Priv *const restrict priv,
	register const uint nchan
)
/*@modifies	*priv@*/
{
	MEMSET(&priv->bitcache, 0x00, sizeof priv->bitcache);
	codec_init((struct Codec *) &priv->codec, nchan);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
