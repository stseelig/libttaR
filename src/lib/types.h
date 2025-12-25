#ifndef H_TTA_CODEC_TYPES_H
#define H_TTA_CODEC_TYPES_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/types.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "./common.h"

/* //////////////////////////////////////////////////////////////////////// */

/* alignment necessary for SIMD intrinsics */
#define LIBTTAr_CODECSTATE_PRIV_ALIGN	SIZE_C(16)

/* ======================================================================== */

enum LibTTAr_SampleBytes {
	LIBTTAr_SAMPLEBYTES_1	= 1u,
	LIBTTAr_SAMPLEBYTES_2	= 2u,
	LIBTTAr_SAMPLEBYTES_3	= 3u
};
#define LIBTTAr_SAMPLEBYTES_MAX	((unsigned int) LIBTTAr_SAMPLEBYTES_3)
#define LIBTTAr_SAMPLEBITS_MAX	((unsigned int) ( \
	8u * LIBTTAr_SAMPLEBYTES_MAX \
))

#define SAMPLEBYTES_RANGE_ASSERT(x_samplebytes) { \
	assert(((x_samplebytes) >= LIBTTAr_SAMPLEBYTES_1) \
	      && \
	       ((x_samplebytes) <= LIBTTAr_SAMPLEBYTES_3) \
	); \
}

/* ======================================================================== */

#ifndef LIBTTAr_OPT_FEWER_FAST_TYPES

/* AMD Ryzen 7 1700 */
typedef uint_fast8_t	bitcnt;
typedef uint32_t	rice24_enc;
typedef uint_fast32_t	rice24_dec;
typedef uint32_t	crc32_enc;
typedef uint_fast32_t	crc32_dec;
typedef uint_fast32_t	cache32;
typedef uint_fast64_t	cache64;
typedef rice24_enc	bitcnt_enc;
typedef bitcnt		bitcnt_dec;

#else	/* defined(LIBTTAr_OPT_FEWER_FAST_TYPES) */

/* Intel Celeron N2830 */
typedef uint_fast8_t	bitcnt;
typedef uint32_t	rice24_enc;
typedef uint32_t	rice24_dec;
typedef uint32_t	crc32_enc;
typedef uint32_t	crc32_dec;
typedef uint32_t	cache32;
typedef uint_fast64_t	cache64;
typedef rice24_enc	bitcnt_enc;
typedef bitcnt		bitcnt_dec;

#endif	/* LIBTTAr_OPT_ONLY_NECESSARY_FAST_TYPES */

/* ======================================================================== */

struct BitCache_Enc {
	cache64		cache;
	bitcnt_enc	count;
};

struct BitCache_Dec {
	cache32		cache;
	bitcnt_dec	count;
};

union BitCache {
	struct BitCache_Enc	enc;
	struct BitCache_Dec	dec;
};

struct Rice_Enc {
	uint32_t	sum[2u];	/* needs 32-bit wrapping */
	bitcnt_enc	k[2u];
};

struct Rice_Dec {
	uint32_t	sum[2u];	/* needs 32-bit wrapping */
	bitcnt_dec	k[2u];
};

union Rice {
	struct Rice_Enc	enc;
	struct Rice_Dec	dec;
};

/* struct Filter */
#include "./filter/struct.h"

struct ALIGNED(LIBTTAr_CODECSTATE_PRIV_ALIGN) Codec {
	struct Filter	filter;
	union Rice	rice;
	int32_t		prev;
};

struct ALIGNED(LIBTTAr_CODECSTATE_PRIV_ALIGN) LibTTAr_CodecState_Priv {
	union BitCache	bitcache;
	struct Codec 	codec[];
};

struct LibTTAr_CodecState_User {
	uint32_t	ncalls_codec;
	uint32_t	crc;
	size_t		ni32;		 /* enc: num read, dec: num written */
	size_t		ni32_total;	 /* ~                               */
	size_t		nbytes_tta;	 /* enc: num written, dec: num read */
	size_t		nbytes_tta_total;/* ~                               */
};

struct LibTTAr_EncMisc {
	size_t				dest_len;
	size_t				src_len;
	size_t				ni32_target;
	size_t				ni32_perframe;
	enum LibTTAr_SampleBytes	samplebytes;
	unsigned int			nchan;
};

struct LibTTAr_DecMisc {
	size_t				dest_len;
	size_t				src_len;
	size_t				ni32_target;
	size_t				nbytes_tta_target;
	size_t				ni32_perframe;
	size_t				nbytes_tta_perframe;
	enum LibTTAr_SampleBytes	samplebytes;
	unsigned int			nchan;
};

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_TYPES_H */
