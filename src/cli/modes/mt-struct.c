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

#include "../debug.h"
#include "../formats.h"

#include "mt-struct.h"
#include "pqueue.h"

//////////////////////////////////////////////////////////////////////////////

void
encmt_fstat_init(
	/*@out@*/ struct FileStats_EncMT *const restrict fstat_c,
	const struct FileStats *const restrict fstat
)
/*@modifies	*fstat_c@*/
{
	fstat_c->nchan			= (uint) fstat->nchan;
	fstat_c->samplebytes		= fstat->samplebytes;
	fstat_c->nsamples_perframe	= fstat->framelen;
	fstat_c->decpcm_size		= fstat->decpcm_size;
	return;
}

//==========================================================================//

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
			&io->frames.encbuf[i], samplebuf_len, fstat->nchan,
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
	encoder->frames.navailable	=  io->frames.navailable;
	//
	t.d = pthread_spin_init(
		&encoder->frames.queue.lock, PTHREAD_PROCESS_PRIVATE
	);
	if UNLIKELY ( t.d != 0 ){
		error_sys(t.d, "pthread_spin_init", NULL);
	}
	pqueue_init(&encoder->frames.queue.q, framequeue_len);
	//
	encoder->frames.post_encoder	=  io->frames.post_encoder;
	encoder->frames.ni32_perframe	=  io->frames.ni32_perframe;
	encoder->frames.encbuf		=  io->frames.encbuf;
	encoder->frames.user		=  io->frames.user;

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

// EOF ///////////////////////////////////////////////////////////////////////
