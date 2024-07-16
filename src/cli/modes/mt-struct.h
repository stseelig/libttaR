#ifndef TTA_MODES_MT_STRUCT_H
#define TTA_MODES_MT_STRUCT_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mt-struct.h                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
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
	uint			nchan;
	enum TTASampleBytes	samplebytes;
	size_t			nsamples_perframe;
	size_t			decpcm_size;
};

struct FileStats_DecMT {
	uint			nchan;
	enum TTASampleBytes	samplebytes;
	size_t			nsamples_perframe;
	size_t			nsamples_enc;
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
	/*@owned@*/
	semaphore_p			*post_encoder;
	/*@owned@*/
	size_t				*ni32_perframe;
	/*@owned@*/
	struct EncBuf			*encbuf;
	/*@owned@*/
	struct LibTTAr_CodecState_User	*user;
};

struct MTArg_Encoder_Frames {
	/*@dependent@*/
	semaphore_p			*navailable;

	struct MTPQueue			queue;

	// parallel arrays
	/*@dependent@*/
	semaphore_p			*post_encoder;
	/*@dependent@*/
	const size_t			*ni32_perframe;
	/*@dependent@*/
	struct EncBuf			*encbuf;
	/*@dependent@*/
	struct LibTTAr_CodecState_User	*user;
};

//--------------------------------------------------------------------------//

struct MTArg_DecIO_Frames {
	uint				nmemb;
	/*@owned@*/
	semaphore_p			*navailable;

	// parallel arrays
	/*@owned@*/
	semaphore_p			*post_decoder;
	/*@owned@*/
	size_t				*ni32_perframe;
	/*@owned@*/
	size_t				*nbytes_tta_perframe;
	/*@owned@*/
	struct DecBuf			*decbuf;
	/*@owned@*/
	u32				*crc_read;	// little-endian
	/*@owned@*/
	struct LibTTAr_CodecState_User	*user;
	/*@owned@*/
	ichar				*dec_retval;
	/*@owned@*/
	size_t				*nsamples_flat_2pad;
};

struct MTArg_Decoder_Frames {
	/*@dependent@*/
	semaphore_p			*navailable;

	struct MTPQueue			queue;

	// parallel arrays
	/*@dependent@*/
	semaphore_p			*post_decoder;
	/*@dependent@*/
	size_t				*ni32_perframe;
	/*@dependent@*/
	size_t				*nbytes_tta_perframe;
	/*@dependent@*/
	struct DecBuf			*decbuf;
	/*@dependent@*/
	struct LibTTAr_CodecState_User	*user;
	/*@dependent@*/
	ichar				*dec_retval;
	/*@dependent@*/
	size_t				*nsamples_flat_2pad;
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
	/*@out@*/ register struct FileStats_EncMT *restrict fstat_c,
	register const struct FileStats *restrict
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
		io->frames.post_encoder,
		io->frames.ni32_perframe,
		io->frames.encbuf,
		io->frames.encbuf[].i32buf,
		io->frames.encbuf[].ttabuf,
		io->frames.user
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
		io->frames.post_encoder,
		io->frames.ni32_perframe,
		io->frames.encbuf[].i32buf,
		io->frames.encbuf[].ttabuf,
		io->frames.encbuf,
		io->frames.user
@*/
;

//--------------------------------------------------------------------------//

#undef fstat_c
INLINE void decmt_fstat_init(
	/*@out@*/ register struct FileStats_DecMT *restrict fstat_c,
	register const struct FileStats *restrict
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
		io->frames.post_decoder,
		io->frames.ni32_perframe,
		io->frames.nbytes_tta_perframe,
		io->frames.decbuf,
		io->frames.decbuf[].pcmbuf,
		io->frames.decbuf[].ttabuf,
		io->frames.crc_read,
		io->frames.user,
		io->frames.dec_retval,
		io->frames.nsamples_flat_2pad
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
		io->frames.post_decoder,
		io->frames.ni32_perframe,
		io->frames.nbytes_tta_perframe,
		io->frames.decbuf[].pcmbuf,
		io->frames.decbuf[].ttabuf,
		io->frames.decbuf,
		io->frames.crc_read,
		io->frames.user,
		io->frames.dec_retval,
		io->frames.nsamples_flat_2pad
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
	/*@out@*/ register struct FileStats_EncMT *restrict fstat_c,
	register const struct FileStats *restrict fstat
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
	/*@out@*/ register struct FileStats_DecMT *restrict fstat_c,
	register const struct FileStats *restrict fstat
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
