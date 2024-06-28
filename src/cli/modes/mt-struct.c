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

/**@fn encmt_state_init
 * @brief initializes the multi-threaded encoder state structs
 *
 * @param io[out] state struct for the io thread
 * @param encoder[out] state struct for the encoder threads
 * @param framequeue_len length of the framequeue
 * @param i32buf_len length of the encbuf->i32buf
 * @param outfile the destination file
 * @param outfile_name the name of the destination file (warnings/errors)
 * @param infile the source file
 * @param infile_name the name of the source file (warnings/errors)
 * @param seektable the TTA seektable struct
 * @param estat_out the encode stats return struct
 * @param fstat the the compacted file stats struct
**/
void
encmt_state_init(
	/*@out@*/ struct MTArg_EncIO *const restrict io,
	/*@out@*/ struct MTArg_Encoder *const restrict encoder,
	const uint framequeue_len, const size_t i32buf_len,
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
		io->frames.encbuf[].i32buf,
		io->frames.encbuf[].ttabuf,
		io->frames.user
@*/
{
	uint i;
	union {	int d; } t;

	// io->frames
	io->frames.nmemb	= framequeue_len;
	//
	io->frames.navailable	= malloc(sizeof *io->frames.navailable);
	if UNLIKELY ( io->frames.navailable == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(io->frames.navailable != NULL);
	t.d = sem_init(io->frames.navailable, 0, 0);
	if UNLIKELY ( t.d != 0 ){
		error_sys(errno, "sem_init", NULL);
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
	        (io->frames.post_encoder	== NULL)
	       ||
	        (io->frames.ni32_perframe	== NULL)
	       ||
	        (io->frames.encbuf		== NULL)
	       ||
	        (io->frames.user		== NULL)
	){
		error_sys(errno, "calloc", NULL);
	}
	assert(io->frames.post_encoder	!= NULL);
	assert(io->frames.ni32_perframe	!= NULL);
	assert(io->frames.encbuf	!= NULL);
	assert(io->frames.user		!= NULL);
	//
	for ( i = 0; i < framequeue_len; ++i ){
		t.d = sem_init(&io->frames.post_encoder[i], 0, 0);
		if UNLIKELY ( t.d != 0 ){
			error_sys(errno, "sem_init", NULL);
		}
	}
	for ( i = 0; i < framequeue_len; ++i ){
		encbuf_init(
			&io->frames.encbuf[i], i32buf_len,
			TTABUF_LEN_DEFAULT, fstat->nchan, fstat->samplebytes
		);
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

/**@fn encmt_state_free
 * @brief frees any allocated pointers and destroys any pthread objects in the
 *   multi-threaded encoder state structs
 *
 * @param io[in] state struct for the io thread
 * @param encoder[in] state struct for the encoder threads
 * @param framequeue_len length of the framequeue
**/
void
encmt_state_free(
	struct MTArg_EncIO *const restrict io,
	struct MTArg_Encoder *const restrict encoder,
	const uint framequeue_len
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
{
	uint i;
	union {	int d; } t;
#ifdef NDEBUG
	(void) t.d;	// gcc
#endif
	// io
	t.d = sem_destroy(io->frames.navailable);
	assert(t.d == 0);
	free(io->frames.navailable);
	for ( i = 0; i < framequeue_len; ++i ){
		t.d = sem_destroy(&io->frames.post_encoder[i]);
		assert(t.d == 0);
	}
	free(io->frames.post_encoder);
	free(io->frames.ni32_perframe);
	for ( i = 0; i < framequeue_len; ++i ){
		codecbuf_free(&io->frames.encbuf[i]);
	}
	free(io->frames.encbuf);
	free(io->frames.user);

	// encoder
	t.d = pthread_spin_destroy(&encoder->frames.queue.lock);
	assert(t.d == 0);

	return;
}

//--------------------------------------------------------------------------//

/**@fn decmt_state_init
 * @brief initializes the multi-threaded decoder state structs
 *
 * @param io[out] state struct for the io thread
 * @param decoder[out] state struct for the decoder threads
 * @param framequeue_len length of the framequeue
 * @param i32buf_len length of the encbuf->i32buf
 * @param outfile the destination file
 * @param outfile_name the name of the destination file (warnings/errors)
 * @param infile the source file
 * @param infile_name the name of the source file (warnings/errors)
 * @param seektable the TTA seektable struct
 * @param dstat_out the decode stats return struct
 * @param fstat the the compacted file stats struct
**/
void
decmt_state_init(
	/*@out@*/ struct MTArg_DecIO *const restrict io,
	/*@out@*/ struct MTArg_Decoder *const restrict decoder,
	const uint framequeue_len, const size_t i32buf_len,
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
		io->frames.decbuf[].pcmbuf,
		io->frames.decbuf[].ttabuf,
		io->frames.crc_read,
		io->frames.user,
		io->frames.dec_retval,
		io->frames.nsamples_flat_2pad
@*/
{
	uint i;
	union {	int d; } t;

	// io->frames
	io->frames.nmemb	= framequeue_len;
	//
	io->frames.navailable	= malloc(sizeof *io->frames.navailable);
	if UNLIKELY ( io->frames.navailable == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(io->frames.navailable != NULL);
	t.d = sem_init(io->frames.navailable, 0, 0);
	if UNLIKELY ( t.d != 0 ){
		error_sys(errno, "sem_init", NULL);
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
	io->frames.nsamples_flat_2pad	= calloc(
		(size_t) framequeue_len, sizeof *io->frames.nsamples_flat_2pad
	);
	if UNLIKELY (
	        (io->frames.post_decoder	== NULL)
	       ||
	        (io->frames.ni32_perframe	== NULL)
	       ||
	        (io->frames.nbytes_tta_perframe	== NULL)
	       ||
	        (io->frames.decbuf		== NULL)
	       ||
	        (io->frames.crc_read		== NULL)
	       ||
	        (io->frames.user		== NULL)
	       ||
	        (io->frames.dec_retval		== NULL)
	       ||
	        (io->frames.nsamples_flat_2pad	== NULL)
	){
		error_sys(errno, "calloc", NULL);
	}
	assert(io->frames.post_decoder		!= NULL);
	assert(io->frames.ni32_perframe		!= NULL);
	assert(io->frames.nbytes_tta_perframe	!= NULL);
	assert(io->frames.decbuf		!= NULL);
	assert(io->frames.crc_read		!= NULL);
	assert(io->frames.user			!= NULL);
	assert(io->frames.dec_retval		!= NULL);
	assert(io->frames.nsamples_flat_2pad	!= NULL);
	//
	for ( i = 0; i < framequeue_len; ++i ){
		t.d = sem_init(&io->frames.post_decoder[i], 0, 0);
		if UNLIKELY ( t.d != 0 ){
			error_sys(errno, "sem_init", NULL);
		}
	}
	for ( i = 0; i < framequeue_len; ++i ){
		decbuf_init(
			&io->frames.decbuf[i], i32buf_len,
			TTABUF_LEN_DEFAULT, fstat->nchan, fstat->samplebytes
		);
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
	decoder->frames.navailable          =  io->frames.navailable;
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
	decoder->frames.nsamples_flat_2pad  = io->frames.nsamples_flat_2pad;

	// decoder other
	decoder->fstat                      = fstat;

	return;
}

/**@fn decmt_state_free
 * @brief frees any allocated pointers and destroys any pthread objects in the
 *   multi-threaded decoder state structs
 *
 * @param io[in] state struct for the io thread
 * @param decoder[in] state struct for the decoder threads
 * @param framequeue_len length of the framequeue
**/
void
decmt_state_free(
	struct MTArg_DecIO *const restrict io,
	struct MTArg_Decoder *const restrict decoder,
	const uint framequeue_len
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
{
	uint i;
	union {	int d; } t;
#ifdef NDEBUG
	(void) t.d;	// gcc
#endif
	// io
	t.d = sem_destroy(io->frames.navailable);
	assert(t.d == 0);
	free(io->frames.navailable);
	for ( i = 0; i < framequeue_len; ++i ){
		t.d = sem_destroy(&io->frames.post_decoder[i]);
		assert(t.d == 0);
	}
	free(io->frames.post_decoder);
	free(io->frames.ni32_perframe);
	free(io->frames.nbytes_tta_perframe);
	for ( i = 0; i < framequeue_len; ++i ){
		codecbuf_free((struct EncBuf *) &io->frames.decbuf[i]);
	}
	free(io->frames.decbuf);
	free(io->frames.crc_read);
	free(io->frames.user);
	free(io->frames.dec_retval);
	free(io->frames.nsamples_flat_2pad);

	// decoder
	t.d = pthread_spin_destroy(&decoder->frames.queue.lock);
	assert(t.d == 0);

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
