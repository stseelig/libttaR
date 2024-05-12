#ifndef TTA_TTA2_H
#define TTA_TTA2_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// tta2.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t
#include <stdio.h>	// FILE

#include "../bits.h"
#include "../libttaR.h"

#include "bufs.h"
#include "formats.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////////

#define FRAMEQUEUE_LEN	( \
	g_framequeue_len != 0 ? g_framequeue_len : 2u*g_nthreads \
)

//////////////////////////////////////////////////////////////////////////////

INLINE size_t enc_readlen(
	register size_t, register size_t, register size_t,
	register enum TTASampleBytes, register uint
)
/*@*/
;

//--------------------------------------------------------------------------//

#undef buf
INLINE uint encst_frame_zeropad(
	register i32 *const restrict buf, register size_t, register uint,
	register uint
)
/*@modifies	*buf@*/
;

#undef buf
INLINE uint encmt_frame_zeropad(
	register u8 *const restrict buf, register size_t, register uint,
	register enum TTASampleBytes, register uint
)
/*@modifies	*buf@*/
;

#undef buf
INLINE uint dec_frame_zeropad(
	register i32 *const restrict buf, register size_t, register uint,
	register uint
)
/*@modifies	*buf@*/
;

//==========================================================================//

// tta2dec_st

#undef seektable
#undef dstat_out
#undef outfile
#undef infile
extern void ttadec_loop_st(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct DecStats *const restrict dstat_out,
	const struct FileStats *const restrict,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*seektable,
		*dstat_out,
		outfile,
		infile
@*/
;

//--------------------------------------------------------------------------//

// tta2enc_st.c

#undef seektable
#undef estat_out
#undef outfile
#undef infile
extern void ttaenc_loop_st(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct EncStats *const restrict estat_out,
	const struct FileStats *const restrict,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*seektable,
		*estat_out,
		outfile,
		infile
@*/
;

//--------------------------------------------------------------------------//

// tta2enc_mt.c

#undef seektable
#undef estat
#undef outfile
#undef infile
extern void ttaenc_loop_mt(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct EncStats *const restrict estat_out,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*seektable,
		*estat_out,
		outfile,
		infile
@*/
;

//////////////////////////////////////////////////////////////////////////////

INLINE size_t
enc_readlen(
	register size_t nsamples_perframe, register size_t nsamples_encoded,
	register size_t decpcm_size, register enum TTASampleBytes samplebytes,
	register uint nchan
)
/*@*/
{
	register size_t r = nsamples_perframe * nchan;
	register union { size_t z; } t;

	t.z = (size_t) ((nsamples_encoded + r) * samplebytes);
	if ( t.z > decpcm_size ){
		r  = decpcm_size;
		r -= (size_t) (nsamples_encoded * samplebytes);
		r /= (size_t) samplebytes;
	}
	return r;
}

//--------------------------------------------------------------------------//

// returns nmemb of buf zero-padded
INLINE uint
encst_frame_zeropad(
	register i32 *const restrict buf, register size_t nmemb_read,
	register uint diff, register uint nchan
)
/*@modifies	*buf@*/
{
	register const size_t r = (size_t) (nchan - diff);
	memset(&buf[nmemb_read], 0x00, r * (sizeof *buf));
	return (uint) r;
}

// returns nmemb of buf zero-padded
INLINE uint
encmt_frame_zeropad(
	register u8 *const restrict buf, register size_t nmemb_read,
	register uint diff, register enum TTASampleBytes samplebytes,
	register uint nchan
)
/*@modifies	*buf@*/
{
	register const size_t r   = (size_t) (nchan - diff);
	register const size_t ind = (size_t) (nmemb_read * samplebytes);
	memset(&buf[ind], 0x00, (size_t) (r * samplebytes));
	return (uint) r;
}

// returns nmemb of buf zero-padded
INLINE uint
dec_frame_zeropad(
	register i32 *const restrict buf, register size_t ni32,
	register uint diff, register uint nchan
)
/*@modifies	*buf@*/
{
	register const size_t r = (size_t) (nchan - diff);
	memset(&buf[ni32], 0x00, r * (sizeof *buf));
	return (uint) r;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
