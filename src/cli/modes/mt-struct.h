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

#include "../formats.h"

#include "bufs.h"
#include "pqueue.h"

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
	size_t			enctta_size;
	size_t			nsamples_enc;
};

//==========================================================================//

struct MTPQueue {
	pthread_spinlock_t	lock;
	struct PQueue		q;
};

//--------------------------------------------------------------------------//

struct MTArg_EncIO_Frames {
	uint				nmemb;
	/*@owned@*/
	sem_t				*navailable;

	// parallel arrays
	/*@owned@*/
	sem_t				*post_encoder;
	/*@owned@*/
	size_t				*ni32_perframe;
	/*@owned@*/
	struct EncBuf			*encbuf;
	/*@owned@*/
	struct LibTTAr_CodecState_User	*user;
};

struct MTArg_Encoder_Frames {
	/*@dependent@*/
	sem_t				*navailable;

	struct MTPQueue			queue;

	// parallel arrays
	/*@dependent@*/
	sem_t				*post_encoder;
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
	sem_t				*navailable;

	// parallel arrays
	/*@owned@*/
	sem_t				*post_decoder;
	/*@owned@*/
	size_t				*ni32_perframe;
	/*@owned@*/
	size_t				*nbytes_tta_perframe;
	/*@owned@*/
	struct DecBuf			*decbuf;
	/*@owned@*/
	u32				*crc_read;
	/*@owned@*/
	struct LibTTAr_CodecState_User	*user;
};

struct MTArg_Decoder_Frames {
	/*@dependent@*/
	sem_t				*navailable;

	struct MTPQueue			queue;

	// parallel arrays
	/*@dependent@*/
	sem_t				*post_decoder;
	/*@dependent@*/
	size_t				*ni32_perframe;
	/*@dependent@*/
	size_t				*nbytes_tta_perframe;
	/*@dependent@*/
	struct DecBuf			*decbuf;
	/*@dependent@*/
	struct LibTTAr_CodecState_User	*user;
};

//--------------------------------------------------------------------------//

struct MTArg_IO_Outfile {
	/*@temp@*/
	FILE		*fh;
	/*@temp@*/
	const char	*name;
};

struct MTArg_IO_Infile {
	/*@temp@*/
	FILE		*fh;
	/*@temp@*/
	const char	*name;
};

//==========================================================================//

struct MTArg_EncIO {
	struct MTArg_EncIO_Frames	frames;
	struct MTArg_IO_Outfile		outfile;
	struct MTArg_IO_Infile 		infile;
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
	struct MTArg_IO_Outfile		outfile;
	struct MTArg_IO_Infile 		infile;
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
extern void encmt_fstat_init(
	/*@out@*/ struct FileStats_EncMT *const restrict fstat_c,
	const struct FileStats *const restrict
)
/*@modifies	*fstat_c@*/
;

#undef io
#undef encoder
extern void encmt_state_init(
	/*@out@*/ struct MTArg_EncIO *const restrict io,
	/*@out@*/ struct MTArg_Encoder *const restrict encoder,
	uint, size_t, const FILE *const restrict, const char *const,
	const FILE *const restrict, const char *const,
	const struct SeekTable *const restrict,
	const struct EncStats *const restrict,
	const struct FileStats_EncMT *const restrict
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
		io->frames.user
@*/
;

#undef io
#undef encoder
extern void encmt_state_free(
	struct MTArg_EncIO *const restrict io,
	struct MTArg_Encoder *const restrict encoder, uint
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
/*@releases	io->frames.navailable,
		io->frames.post_encoder,
		io->frames.ni32_perframe,
		io->frames.encbuf,
		io->frames.user
@*/
;

//--------------------------------------------------------------------------//



// EOF ///////////////////////////////////////////////////////////////////////
#endif
