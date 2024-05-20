//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mt-struct.c                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdlib.h>

#include <pthread.h>
#include <semaphore.h>

#include "../../bits.h"
#include "../../libttaR.h"	// sizeof *user

#include "../debug.h"
#include "../formats.h"

#include "mt-struct.h"
#include "pqueue.h"

//////////////////////////////////////////////////////////////////////////////

void
encmt_state_init(
	/*@out@*/ struct MTArg_EncIO *const restrict io,
	/*@out@*/ struct MTArg_Encoder *const restrict encoder,
	uint framequeue_len, size_t samplebuf_len,
	const FILE *const restrict outfile, const char *const outfile_name,
	const FILE *const restrict infile, const char *const infile_name,
	const struct SeekTable *const restrict seektable,
	const struct EncStats *const restrict estat_out,
	const struct FileStats_EncMT *const restrict fstat
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
{
	uint i;
	union {	size_t	z;
		int	d;
	} t;

	// io->frames
	io->frames.nmemb	= framequeue_len;
	//
	io->frames.navailable	= malloc(sizeof *io->frames.navailable);
	if UNLIKELY ( io->frames.navailable == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(io->frames.navailable != NULL);
	t.d = sem_init(io->frames.navailable, 0, (uint) 0);
	if UNLIKELY ( t.d != 0 ){
		error_sys(t.d, "sem_init", NULL);
	}
	//
	io->frames.post_encoder		= calloc(
		(size_t) framequeue_len, sizeof *io->frames.post_encoder
	);
	io->frames.ni32_perframe	= calloc(
		(size_t) framequeue_len, sizeof *io->frames.ni32_perframe
	);
	io->frames.encbuf		= calloc(
		(size_t) framequeue_len, sizeof *io->frames.encbuf
	);
	io->frames.user			= calloc(
		(size_t) framequeue_len, sizeof *io->frames.user
	);
	if UNLIKELY (
	        (io->frames.post_encoder == NULL)
	       ||
	        (io->frames.ni32_perframe == NULL)
	       ||
	        (io->frames.encbuf == NULL)
	       ||
	        (io->frames.user == NULL)
	){
		error_sys(errno, "calloc", NULL);
	}
	assert(io->frames.post_encoder != NULL);
	assert(io->frames.ni32_perframe != NULL);
	assert(io->frames.encbuf != NULL);
	assert(io->frames.user != NULL);
	//
	for ( i = 0; i < framequeue_len; ++i ){
		t.d = sem_init(&io->frames.post_encoder[i], 0, 0);
		if UNLIKELY ( t.d != 0 ){
			error_sys(t.d, "sem_init", NULL);
		}
	}
	for ( i = 0; i < framequeue_len; ++i ){
		t.z = encbuf_init(
			&io->frames.encbuf[i], samplebuf_len,
			SAMPLEBUF_LEN_DEFAULT, fstat->nchan,
			fstat->samplebytes
		);
		assert(t.z == samplebuf_len * fstat->nchan);
	}

	// io->outfile
	io->outfile.fh		= (FILE *) outfile;
	io->outfile.name	= outfile_name;

	// io->infile
	io->infile.fh		= (FILE *) infile;
	io->infile.name		= infile_name;

	// io other
	io->fstat		= fstat;
	io->seektable		= (struct SeekTable *) seektable;
	io->estat_out		= (struct EncStats *) estat_out;

	// encoder->frames
	encoder->frames.navailable	= io->frames.navailable;
	//
	t.d = pthread_spin_init(
		&encoder->frames.queue.lock, PTHREAD_PROCESS_PRIVATE
	);
	if UNLIKELY ( t.d != 0 ){
		error_sys(t.d, "pthread_spin_init", NULL);
	}
	pqueue_init(&encoder->frames.queue.q, framequeue_len);
	//
	encoder->frames.post_encoder	= io->frames.post_encoder;
	encoder->frames.ni32_perframe	= io->frames.ni32_perframe;
	encoder->frames.encbuf		= io->frames.encbuf;
	encoder->frames.user		= io->frames.user;

	// encoder other
	encoder->fstat			= fstat;

	return;
}

void
encmt_state_free(
	struct MTArg_EncIO *const restrict io,
	struct MTArg_Encoder *const restrict encoder, uint framequeue_len
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
{
	uint i;

	// io
	(void) sem_destroy(io->frames.navailable);
	free(io->frames.navailable);
	for ( i = 0; i < framequeue_len; ++i ){
		(void) sem_destroy(&io->frames.post_encoder[i]);
	}
	free(io->frames.post_encoder);
	free(io->frames.ni32_perframe);
	for ( i = 0; i < framequeue_len; ++i ){
		encbuf_free(&io->frames.encbuf[i]);
	}
	free(io->frames.encbuf);
	free(io->frames.user);

	// encoder
	(void) pthread_spin_destroy(&encoder->frames.queue.lock);

	return;
}

//--------------------------------------------------------------------------//

void
decmt_state_init(
	/*@out@*/ struct MTArg_DecIO *const restrict io,
	/*@out@*/ struct MTArg_Decoder *const restrict decoder,
	uint framequeue_len, size_t samplebuf_len,
	const FILE *const restrict outfile, const char *const outfile_name,
	const FILE *const restrict infile, const char *const infile_name,
	const struct SeekTable *const restrict seektable,
	const struct DecStats *const restrict dstat_out,
	const struct FileStats_DecMT *const restrict fstat
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
		io->frames.crc_read,
		io->frames.user,
		io->frames.dec_retval
@*/
{
	uint i;
	union {	size_t	z;
		int	d;
	} t;

	// io->frames
	io->frames.nmemb	= framequeue_len;
	//
	io->frames.navailable	= malloc(sizeof *io->frames.navailable);
	if UNLIKELY ( io->frames.navailable == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(io->frames.navailable != NULL);
	t.d = sem_init(io->frames.navailable, 0, (uint) 0);
	if UNLIKELY ( t.d != 0 ){
		error_sys(t.d, "sem_init", NULL);
	}
	//
	io->frames.post_decoder		= calloc(
		(size_t) framequeue_len, sizeof *io->frames.post_decoder
	);
	io->frames.ni32_perframe	= calloc(
		(size_t) framequeue_len, sizeof *io->frames.ni32_perframe
	);
	io->frames.nbytes_tta_perframe	= calloc(
		(size_t) framequeue_len,
		sizeof *io->frames.nbytes_tta_perframe
	);
	io->frames.decbuf		= calloc(
		(size_t) framequeue_len, sizeof *io->frames.decbuf
	);
	io->frames.crc_read		= calloc(
		(size_t) framequeue_len, sizeof *io->frames.crc_read
	);
	io->frames.user			= calloc(
		(size_t) framequeue_len, sizeof *io->frames.user
	);
	io->frames.dec_retval		= calloc(
		(size_t) framequeue_len, sizeof *io->frames.dec_retval
	);
	if UNLIKELY (
	        (io->frames.post_decoder == NULL)
	       ||
	        (io->frames.ni32_perframe == NULL)
	       ||
	        (io->frames.nbytes_tta_perframe == NULL)
	       ||
	        (io->frames.decbuf == NULL)
	       ||
	        (io->frames.crc_read == NULL)
	       ||
	        (io->frames.user == NULL)
	       ||
	        (io->frames.dec_retval == NULL)
	){
		error_sys(errno, "calloc", NULL);
	}
	assert(io->frames.post_decoder != NULL);
	assert(io->frames.ni32_perframe != NULL);
	assert(io->frames.nbytes_tta_perframe != NULL);
	assert(io->frames.decbuf != NULL);
	assert(io->frames.crc_read != NULL);
	assert(io->frames.user != NULL);
	assert(io->frames.dec_retval != NULL);
	//
	for ( i = 0; i < framequeue_len; ++i ){
		t.d = sem_init(&io->frames.post_decoder[i], 0, 0);
		if UNLIKELY ( t.d != 0 ){
			error_sys(t.d, "sem_init", NULL);
		}
	}
	for ( i = 0; i < framequeue_len; ++i ){
		t.z = decbuf_init(
			&io->frames.decbuf[i], samplebuf_len,
			SAMPLEBUF_LEN_DEFAULT, fstat->nchan,
			fstat->samplebytes
		);
		assert(t.z == samplebuf_len * fstat->nchan);
	}

	// io->outfile
	io->outfile.fh		= (FILE *) outfile;
	io->outfile.name	= outfile_name;

	// io->infile
	io->infile.fh		= (FILE *) infile;
	io->infile.name		= infile_name;

	// io other
	io->fstat		= fstat;
	io->seektable		= (struct SeekTable *) seektable;
	io->dstat_out		= (struct DecStats *) dstat_out;

	// decoder->frames
	decoder->frames.navailable	=  io->frames.navailable;
	//
	t.d = pthread_spin_init(
		&decoder->frames.queue.lock, PTHREAD_PROCESS_PRIVATE
	);
	if UNLIKELY ( t.d != 0 ){
		error_sys(t.d, "pthread_spin_init", NULL);
	}
	pqueue_init(&decoder->frames.queue.q, framequeue_len);
	//
	decoder->frames.post_decoder        = io->frames.post_decoder;
	decoder->frames.ni32_perframe       = io->frames.ni32_perframe;
	decoder->frames.nbytes_tta_perframe = io->frames.nbytes_tta_perframe;
	decoder->frames.decbuf              = io->frames.decbuf;
	decoder->frames.user                = io->frames.user;
	decoder->frames.dec_retval          = io->frames.dec_retval;

	// decoder other
	decoder->fstat			= fstat;

	return;
}

void
decmt_state_free(
	struct MTArg_DecIO *const restrict io,
	struct MTArg_Decoder *const restrict decoder, uint framequeue_len
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
/*@releases	io->frames.navailable,
		io->frames.post_decoder,
		io->frames.ni32_perframe,
		io->frames.nbytes_tta_perframe,
		io->frames.decbuf,
		io->frames.crc_read,
		io->frames.user,
		io->frames.dec_retval
@*/
{
	uint i;

	// io
	(void) sem_destroy(io->frames.navailable);
	free(io->frames.navailable);
	for ( i = 0; i < framequeue_len; ++i ){
		(void) sem_destroy(&io->frames.post_decoder[i]);
	}
	free(io->frames.post_decoder);
	free(io->frames.ni32_perframe);
	free(io->frames.nbytes_tta_perframe);
	for ( i = 0; i < framequeue_len; ++i ){
		decbuf_free(&io->frames.decbuf[i]);
	}
	free(io->frames.decbuf);
	free(io->frames.crc_read);
	free(io->frames.user);
	free(io->frames.dec_retval);

	// decoder
	(void) pthread_spin_destroy(&decoder->frames.queue.lock);

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
