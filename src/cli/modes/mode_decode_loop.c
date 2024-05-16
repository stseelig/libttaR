//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mode_decode_loop.c                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	// memset

#include <pthread.h>
#include <semaphore.h>

#include "../../bits.h"
#include "../../libttaR.h"
#include "../../splint.h"

#include "../cli.h"
#include "../debug.h"
#include "../formats.h"
#include "../main.h"

#include "bufs.h"
#include "mt-struct.h"
#include "pqueue.h"

//////////////////////////////////////////////////////////////////////////////



//////////////////////////////////////////////////////////////////////////////

// can deadlock or abort if (framequeue_len <= nthreads)
void
decmt_loop(
	const struct SeekTable *const restrict seektable,
	/*@out@*/ struct DecStats *const restrict dstat_out,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name,
	uint nthreads
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*dstat_out,
		outfile,
		infile
@*/
{
	struct MTArg_DecIO state_io;
	struct MTArg_Decoder state_decoder;
	pthread_t *thread_decoder = NULL;
	struct FileStats_DecMT fstat_c;
	const size_t samplebuf_len = fstat->buflen;
	const uint framequeue_len = FRAMEQUEUE_LEN(nthreads);
	uint i;
	union {	int d; } t;

	// init
	decmt_fstat_init(&fstat_c, fstat);
	//
	decmt_state_init(
		&state_io, &state_decoder, framequeue_len, samplebuf_len,
		outfile, outfile_name, infile, infile_name, seektable,
		dstat_out, &fstat_c
	);
	//
	thread_decoder = calloc((size_t) nthreads, sizeof *thread_decoder);
	assert(thread_decoder != NULL);
	if UNLIKELY ( thread_decoder == NULL ){
		error_sys(errno, "calloc", NULL);
	}

	// create
	for ( i = 0; i < nthreads; ++i ){
		t.d = pthread_create(
			&thread_decoder[i], NULL,
			(void *(*)(void *)) decmt_decoder, &state_decoder
		);
		if UNLIKELY ( t.d != 0 ){
			error_sys(t.d, "pthread_create", NULL);
		}
	}

	// the main thread does the file-io
	(void) decmt_io(&state_io);

	// join
	for ( i = 0; i < nthreads; ++i ){
		(void) pthread_join(thread_decoder[i], NULL);
	}

	// cleanup
	decmt_state_free(&state_io, &state_decoder, framequeue_len);
	free(thread_decoder);

	return;
}

//==========================================================================//



//==========================================================================//

/*@null@*/
static void *
decmt_io(struct MTArg_DecIO *const restrict arg)
// TODO
{
	struct MTArg_DecIO_Frames *const restrict  frames  = &arg->frames;
	struct MTArg_IO_Outfile   *const restrict  outfile = &arg->outfile;
	struct MTArg_IO_Infile    *const restrict  infile  = &arg->infile;
	const struct FileStats_DecMT *const restrict fstat =  arg->fstat;
	const struct SeekTable *const restrict seektable   =  arg->seektable;
	//
	sem_t         *const restrict nframes_avail =  frames->navailable;
	sem_t         *const restrict post_encoder  =  frames->post_encoder;
	size_t        *const restrict ni32_perframe =  frames->ni32_perframe;
	size_t        *const restrict nbytes_tta_perframe   = (
		frames->nbytes_tta_perframe
	);
	struct EncBuf *const restrict encbuf        =  frames->encbuf;
	struct LibTTAr_CodecState_User *const restrict user = frames->user;
	//
	FILE       *const restrict outfile_fh   = outfile->fh;
	const char *const restrict outfile_name = outfile->name;
	FILE       *const restrict infile_fh    = infile->fh;
	const char *const restrict infile_name  = infile->name;
	//
	const uint framequeue_len             = frames->nmemb;
	const uint nchan                      = fstat->nchan;
	const enum TTASampleBytes samplebytes = fstat->samplebytes;
	const size_t nsamples_perframe        = fstat->nsamples_perframe;
	const size_t enctta_size              = fstat->encpcm_size;
	const size_t nsamples_enc             = fstat->nsamples_enc;

	struct DecStats dstat;
	size_t readlen, nmemb_read;
	size_t nsamples_flat_read_total = 0;
	bool start_writing = false, truncated = false;
	size_t framecnt = 0;
	uint i = 0, last;
	union {	uint u; } t;

	memset(&dstat, 0x00, sizeof dstat);
	goto loop0_entr;
	do {
		++framecnt;
		i = (i + 1u < framequeue_len ? i + 1u : 0);
		if ( ! start_writing ){
			if ( i == 0 ){
				start_writing = true;
			}
			else {	goto loop0_read; }
		}

		// check for truncated sample

		// wait for frame to finish encoding
		(void) sem_wait(&post_decoder[i]);

		// write pcm to outfile
		dec_frame_write(
			&decbuf[i], seektable, &dstat, &user[i], outfile_fh,
			outfile_name, nchan
		);
loop0_entr:
		if ( (! g_flag.quiet) && (framecnt % SPINNER_FRQ == 0) ){
			errprint_spinner();
		}
loop0_read:
		// read tta from infile

		// make frame available

	} while ( ! truncated );

	*arg->dstat_out = dstat;
	return NULL;
}

/*@null@*/
static void *
decmt_decoder(struct MTArg_Decoder *const restrict arg)
// TODO
{
	struct MTArg_Decoder_Frames  *const restrict frames = &arg->frames;
	const struct FileStats_DecMT *const restrict fstat  =  arg->fstat;
	//
	sem_t        *const restrict nframes_avail =  frames->navailable;
	struct MTPQueue *const restrict queue      = &frames->queue;
	sem_t        *const restrict post_decoder  =  frames->post_decoder;
	const size_t *const restrict ni32_perframe =  frames->ni32_perframe;
	const size_t *const restrict nbytes_tta_perframe    = (
		frames->nbytes_tta_perframe
	);
	struct DecBuf *const restrict decbuf       =  frames->decbuf;
	struct LibTTAr_CodecState_User *const restrict user = frames->user;
	//
	const uint nchan                      = fstat->nchan;
	const enum TTASampleBytes samplebytes = fstat->samplebytes;

	struct LibTTAr_CodecState_Priv *restrict priv;
	uint i;
	union {	size_t z; } t;

	// setup
	t.z = libttaR_codecstate_priv_size(nchan);
	assert(t.z != 0);
	priv = malloc(t.z);
	if UNLIKELY ( priv == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(priv != NULL);

	goto loop_entr;
	do {
		// decode frame
		dec_frame_decode(
			&decbuf[i], priv, &user[i], samplebytes, nchan,
			ni32_perframe[i], nbytes_tta_perframe[i]
		);

		// unlock frame
		(void) sem_post(&post_decoder[i]);
loop_entr:
		// wait for a frame to be available
		(void) sem_wait(nframes_avail);

		// get frame id from decode queue
		(void) pthread_spin_lock(&queue->lock);
		i = pdequeue(&queue->q);
		(void) pthread_spin_unlock(&queue->lock);
	}
	while ( nbytes_tta_perframe[i] != 0 );

	// cleanup
	free(priv);

	return NULL;
}

// EOF ///////////////////////////////////////////////////////////////////////
