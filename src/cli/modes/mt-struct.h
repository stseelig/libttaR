#ifndef TTA_MODES_MT_STRUCT_H
#define TTA_MODES_MT_STRUCT_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mt-struct.h                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>

#include "../../bits.h"
#include "../../libttaR.h"	// struct LibTTAr_CodecState_User

#include "../formats.h"

#include "bufs.h"
#include "pqueue.h"
#include "threads.h"

//////////////////////////////////////////////////////////////////////////////

// multi-threaded ver. can deadlock or abort if (framequeue_len <= nthreads)
#define FRAMEQUEUE_LEN(nthreads)	((uint) (2u*(nthreads)))

//////////////////////////////////////////////////////////////////////////////

struct FileStats_EncMT {
	uint				nchan;
	enum LibTTAr_SampleBytes	samplebytes;
	size_t				nsamples_perframe;
	size_t				decpcm_size;
};

struct FileStats_DecMT {
	uint				nchan;
	enum LibTTAr_SampleBytes	samplebytes;
	size_t				nsamples_perframe;
	size_t				nsamples_enc;
};

//==========================================================================//

struct MTPQueue {
	spinlock_p	lock;
	struct PQueue	q;
};

//--------------------------------------------------------------------------//

struct MTArg_EncIO_Frames {
	uint				nmemb;
	/*@owned@*/
	semaphore_p			*navailable;

	// parallel arrays
	/*@temp@*/
	semaphore_p			*post_encoder;
	/*@temp@*/
	size_t				*ni32_perframe;
	/*@temp@*/
	struct EncBuf			*encbuf;
	/*@temp@*/
	struct LibTTAr_CodecState_User	*user;
};

struct MTArg_Encoder_Frames {
	struct MTPQueue			queue;
	/*@dependent@*/
	semaphore_p			*navailable;

	// parallel arrays
	/*@temp@*/
	semaphore_p			*post_encoder;
	/*@temp@*/
	const size_t			*ni32_perframe;
	/*@temp@*/
	struct EncBuf			*encbuf;
	/*@temp@*/
	struct LibTTAr_CodecState_User	*user;
};

//--------------------------------------------------------------------------//

struct MTArg_DecIO_Frames {
	uint				nmemb;
	/*@owned@*/
	semaphore_p			*navailable;

	// parallel arrays
	/*@temp@*/
	semaphore_p			*post_decoder;
	/*@temp@*/
	size_t				*ni32_perframe;
	/*@temp@*/
	size_t				*nbytes_tta_perframe;
	/*@temp@*/
	struct DecBuf			*decbuf;
	/*@temp@*/
	struct LibTTAr_CodecState_User	*user;
	/*@temp@*/
	size_t				*nsamples_flat_2pad;
	/*@temp@*/
	ichar				*dec_retval;
	/*@temp@*/
	u32				*crc_read;	// little-endian
};

struct MTArg_Decoder_Frames {
	struct MTPQueue			queue;
	/*@dependent@*/
	semaphore_p			*navailable;

	// parallel arrays
	/*@temp@*/
	semaphore_p			*post_decoder;
	/*@temp@*/
	size_t				*ni32_perframe;
	/*@temp@*/
	size_t				*nbytes_tta_perframe;
	/*@temp@*/
	struct DecBuf			*decbuf;
	/*@temp@*/
	struct LibTTAr_CodecState_User	*user;
	/*@temp@*/
	size_t				*nsamples_flat_2pad;
	/*@temp@*/
	ichar				*dec_retval;
};

//--------------------------------------------------------------------------//

struct MTArg_IO_File {
	/*@temp@*/
	FILE		*fh;
	/*@temp@*/
	const char	*name;
};

//==========================================================================//

struct MTArg_EncIO {
	struct MTArg_EncIO_Frames	frames;
	struct MTArg_IO_File		outfile;
	struct MTArg_IO_File 		infile;
	/*@temp@*/
	const struct FileStats_EncMT	*fstat;
	/*@temp@*/
	struct SeekTable 		*seektable;
	/*@temp@*/
	struct EncStats			*estat_out;
};

struct MTArg_Encoder {
	struct MTArg_Encoder_Frames	frames;
	/*@temp@*/
	const struct FileStats_EncMT	*fstat;
};

//--------------------------------------------------------------------------//

struct MTArg_DecIO {
	struct MTArg_DecIO_Frames	frames;
	struct MTArg_IO_File		outfile;
	struct MTArg_IO_File 		infile;
	/*@temp@*/
	const struct FileStats_DecMT	*fstat;
	/*@temp@*/
	const struct SeekTable 		*seektable;
	/*@temp@*/
	struct DecStats			*dstat_out;
};

struct MTArg_Decoder {
	struct MTArg_Decoder_Frames	frames;
	/*@temp@*/
	const struct FileStats_DecMT	*fstat;
};

//////////////////////////////////////////////////////////////////////////////


#undef fstat_c
INLINE void encmt_fstat_init(
	/*@out@*/ struct FileStats_EncMT *restrict fstat_c,
	const struct FileStats *restrict
)
/*@modifies	*fstat_c@*/
;

#undef io
#undef encoder
extern void encmt_state_init(
	/*@out@*/ struct MTArg_EncIO *restrict io,
	/*@out@*/ struct MTArg_Encoder *restrict encoder,
	uint, size_t, const FILE *restrict, const char *,
	const FILE *restrict, const char *, const struct SeekTable *restrict,
	const struct EncStats *restrict,
	const struct FileStats_EncMT *restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*io,
		*io->frames.navailable,
		io->frames.post_encoder[],
		*encoder,
		encoder->frames.queue.lock
@*/
/*@allocates	io->frames.navailable,
		io->frames.encbuf[].pcmbuf,
		io->frames.encbuf[].ttabuf
@*/
;

#undef io
#undef encoder
extern void encmt_state_free(
	struct MTArg_EncIO *restrict io,
	struct MTArg_Encoder *restrict encoder, uint
)
/*@globals	internalState@*/
/*@modifies	internalState,
		*io,
		*io->frames.navailable,
		io->frames.post_encoder[],
		*encoder,
		encoder->frames.queue.lock
@*/
/*@releases	io->frames.navailable,
		io->frames.encbuf[].pcmbuf,
		io->frames.encbuf[].ttabuf
@*/
;

//--------------------------------------------------------------------------//

#undef fstat_c
INLINE void decmt_fstat_init(
	/*@out@*/ struct FileStats_DecMT *restrict fstat_c,
	const struct FileStats *restrict
)
/*@modifies	*fstat_c@*/
;

#undef io
#undef decoder
extern void decmt_state_init(
	/*@out@*/ struct MTArg_DecIO *restrict io,
	/*@out@*/ struct MTArg_Decoder *restrict decoder,
	uint, size_t, const FILE *restrict, const char *,
	const FILE *restrict, const char *, const struct SeekTable *restrict,
	const struct DecStats *restrict,
	const struct FileStats_DecMT *restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*io,
		*io->frames.navailable,
		io->frames.post_decoder[],
		*decoder,
		decoder->frames.queue.lock
@*/
/*@allocates	io->frames.navailable,
		io->frames.decbuf[].pcmbuf,
		io->frames.decbuf[].ttabuf
@*/
;

#undef io
#undef decoder
extern void decmt_state_free(
	struct MTArg_DecIO *restrict io,
	struct MTArg_Decoder *restrict decoder, uint
)
/*@globals	internalState@*/
/*@modifies	internalState,
		*io,
		*io->frames.navailable,
		io->frames.post_decoder[],
		*decoder,
		decoder->frames.queue.lock
@*/
/*@releases	io->frames.navailable,
		io->frames.decbuf[].pcmbuf,
		io->frames.decbuf[].ttabuf
@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn encmt_fstat_init
 * @brief initializes the compact file stats struct for the multi-threaded
 *   encoder
 *
 * @param fstat_c[out] the compact file stats struct
 * @param fstat[in] the bloated file stats struct
**/
INLINE void
encmt_fstat_init(
	/*@out@*/ struct FileStats_EncMT *restrict fstat_c,
	const struct FileStats *restrict fstat
)
/*@modifies	*fstat_c@*/
{
	fstat_c->nchan			= (uint) fstat->nchan;
	fstat_c->samplebytes		= fstat->samplebytes;
	fstat_c->nsamples_perframe	= fstat->framelen;
	fstat_c->decpcm_size		= fstat->decpcm_size;
	return;
}

/**@fn decmt_fstat_init
 * @brief initializes the compact file stats struct for the multi-threaded
 *   decoder
 *
 * @param fstat_c[out] the compact file stats struct
 * @param fstat[in] the bloated file stats struct
**/
INLINE void
decmt_fstat_init(
	/*@out@*/ struct FileStats_DecMT *restrict fstat_c,
	const struct FileStats *restrict fstat
)
/*@modifies	*fstat_c@*/
{
	fstat_c->nchan			= (uint) fstat->nchan;
	fstat_c->samplebytes		= fstat->samplebytes;
	fstat_c->nsamples_perframe	= fstat->framelen;
	fstat_c->nsamples_enc		= fstat->nsamples_enc;
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
