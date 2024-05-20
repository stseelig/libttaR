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

#undef decbuf
#undef priv
#undef user_out
static int dec_frame_decode(
	struct DecBuf *const restrict decbuf,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	enum TTASampleBytes, uint, size_t, size_t
)
/*@modifies	*decbuf->i32buf,
		*decbuf->pcmbuf,
		*priv,
		*user_out
@*/
;

#undef decbuf
#undef dstat_out
#undef outfile
static void dec_frame_write(
	struct DecBuf *const restrict decbuf,
	/*@in@*/ struct DecStats *const restrict dstat_out,
	struct LibTTAr_CodecState_User *const restrict,
	FILE *const restrict outfile, const char *const restrict,
	enum TTASampleBytes, uint, u32, ichar
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*decbuf->pcmbuf,
		*dstat_out,
		outfile
@*/
;

static size_t dec_ni32_perframe(size_t, size_t, size_t, uint) /*@*/;

//--------------------------------------------------------------------------//

#undef arg
/*@null@*/
static void *decmt_io(struct MTArg_DecIO *const restrict arg)
// TODO
;

#undef arg
/*@null@*/
static void *decmt_decoder(struct MTArg_Decoder *const restrict arg)
// TODO
;

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
	if UNLIKELY ( thread_decoder == NULL ){
		error_sys(errno, "calloc", NULL);
	}
	assert(thread_decoder != NULL);

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

// returns what libttaR_tta_decode returned
static int
dec_frame_decode(
	struct DecBuf *const restrict decbuf,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	enum TTASampleBytes samplebytes, uint nchan, size_t ni32_perframe,
	size_t nbytes_tta_perframe
)
/*@modifies	*decbuf->i32buf,
		*decbuf->pcmbuf,
		*priv,
		*user_out
@*/
{
	int r;
	struct LibTTAr_CodecState_User user = LIBTTAr_CODECSTATE_USER_INIT;
	union {	size_t z; } t;
#ifdef NDEBUG
	(void) t.z;	// gcc
#endif
	// decode tta to i32
	r = libttaR_tta_decode(
		decbuf->i32buf, decbuf->ttabuf, decbuf->i32buf_len,
		decbuf->ttabuf_len, ni32_perframe, nbytes_tta_perframe, priv,
		&user, samplebytes, nchan, ni32_perframe, nbytes_tta_perframe
	);
	// with the way the decoding is setup, RET_AGAIN can't happen here
	assert((r == LIBTTAr_RET_DONE) || (r == LIBTTAr_RET_DECFAIL));

	// convert i32 to pcm
	t.z = libttaR_pcm_write(
		decbuf->pcmbuf, decbuf->i32buf, user.ni32, samplebytes
	);
	assert(t.z == user.ni32);

	*user_out = user;
	return r;
}

static void
dec_frame_write(
	struct DecBuf *const restrict decbuf,
	/*@in@*/ struct DecStats *const restrict dstat_out,
	struct LibTTAr_CodecState_User *const restrict user_in,
	FILE *const restrict outfile, const char *const restrict outfile_name,
	enum TTASampleBytes samplebytes, uint nchan, u32 crc_read,
	ichar dec_retval
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*decbuf->pcmbuf,
		*dstat_out,
		outfile
@*/
{
	struct LibTTAr_CodecState_User user = *user_in;
	struct DecStats dstat = *dstat_out;
	union { uint	u;
		size_t	z;
	} t;

	// check if frame is DECFAIL or truncated and zero-pad
	if UNLIKELY ( dec_retval == (ichar) LIBTTAr_RET_DECFAIL ){
		// TODO

		// letoh32 on crc; kept as little-endian
	}
	else {	t.u = user.ni32_total % nchan;
		if UNLIKELY ( t.u != 0 ){
			warning_tta("%s: frame %zu: "
				"last sample truncated, zero-padding",
				outfile_name, dstat.nframes
			);
			// TODO zeropad
			user.ni32       += t.z;
			user.ni32_total += t.z;
		}
	}

	// write frame
	t.z = fwrite(decbuf->pcmbuf, samplebytes, user.ni32_total, outfile);
	if UNLIKELY ( t.z != user.ni32 ){
		error_sys(errno, "fwrite", outfile_name);
	}

	// check frame crc
	if UNLIKELY ( user.crc != crc_read ){
		warning_tta("%s: frame %zu is corrupted; bad crc",
			outfile_name, dstat.nframes
		);
	}
	user.nbytes_tta_total += sizeof crc_read;

	// update dstat
	dstat.nframes          += (size_t) 1u;
	dstat.nsamples_flat    += user.ni32_total;
	dstat.nsamples_perchan += (size_t) (user.ni32_total / nchan);
	dstat.nbytes_decoded   += user.nbytes_tta_total;

	*dstat_out = dstat;
	return;
}


// returns ni32_perframe
static size_t
dec_ni32_perframe(
	size_t nsamples_dec, size_t nsamples_enc, size_t nsamples_perframe,
	uint nchan
)
/*@*/
{
	if ( nsamples_dec + nsamples_perframe > nsamples_enc ){
		return (size_t) ((nsamples_enc - nsamples_dec) * nchan);
	}
	else {	return (size_t) (nsamples_perframe * nchan); }
}

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
	sem_t         *const restrict nframes_avail = frames->navailable;
	sem_t         *const restrict post_decoder  = frames->post_decoder;
	size_t        *const restrict ni32_perframe = frames->ni32_perframe;
	size_t        *const restrict nbytes_tta_perframe   = (
		frames->nbytes_tta_perframe
	);
	struct DecBuf *const restrict decbuf        = frames->decbuf;
	u32           *const restrict crc_read      = frames->crc_read;
	struct LibTTAr_CodecState_User *const restrict user = frames->user;
	ichar         *const restrict dec_retval    = frames->dec_retval;
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
	const size_t enctta_size              = fstat->enctta_size;
	const size_t nsamples_enc             = fstat->nsamples_enc;

	struct DecStats dstat;
	size_t framesize_tta, nbytes_read;
	size_t nbytes_read_total = 0;
	size_t ni32_perframe_calc;
	size_t nsamples_perchan_dec_total = 0;
	bool start_writing = false, truncated = false;
	size_t nframes_read = 0, nframes_written = 0;
	uint i = 0, last;
	union {	uint	u;
		size_t	z;
	} t;

	memset(&dstat, 0x00, sizeof dstat);
	goto loop0_entr;
	do {
		++nframes_read;
		i = (i + 1u < framequeue_len ? i + 1u : 0);
		if ( ! start_writing ){
			if ( i == 0 ){
				start_writing = true;
			}
			else {	goto loop0_read; }
		}

		// wait for frame to finish decoding
		(void) sem_wait(&post_decoder[i]);

		// write pcm to outfile
		dec_frame_write(
			&decbuf[i], &dstat, &user[i], outfile_fh,
			outfile_name, samplebytes, nchan,
			letoh32(crc_read[i]), dec_retval[i]
		);
		++nframes_written;
loop0_entr:
		if ( (! g_flag.quiet) && (nframes_read % SPINNER_FRQ == 0) ){
			errprint_spinner();
		}
loop0_read:
		// read tta from infile
		// ENCFMT_TTA1
		framesize_tta = letoh32(seektable->table[nframes_read]);
		if ( framesize_tta <= (sizeof *crc_read) ){
			// TODO warning
			nbytes_tta_perframe[i] = 0;
			break;
		}
		else {	framesize_tta -= (sizeof *crc_read); }
		ni32_perframe_calc = dec_ni32_perframe(
			nsamples_perchan_dec_total, nsamples_enc,
			nsamples_perframe, nchan
		);
		// MAYBE nbytes_tta_perframe_calc
		decbuf_check_adjust(&decbuf[i], framesize_tta, nchan);
		nbytes_read = fread(
			decbuf[i].ttabuf, (size_t) 1u, framesize_tta,
			infile_fh
		);
		ni32_perframe[i]       = ni32_perframe_calc;
		nbytes_tta_perframe[i] = nbytes_read;
		//
		if UNLIKELY ( nbytes_read != framesize_tta ){
			if UNLIKELY ( ferror(infile_fh) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile_name, nframes_read
				);
			}
			goto loop0_truncated;
		}

		// read frame footer (crc); kept as little-endian
		t.z = fread(
			&crc_read[i], sizeof *crc_read, (size_t) 1u, infile_fh
		);
		if UNLIKELY ( t.z != (size_t) 1u ){
			if UNLIKELY ( ferror(infile_fh) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile_name, nframes_read
				);
			}
loop0_truncated:
			truncated   = true;
			crc_read[i] = 0;
		}

		// make frame available
		(void) sem_post(nframes_avail);
	}
// TODO end condition like encmt
	while ( ! truncated );
	last = i;

	// write the remaining frames
	if ( start_writing ){ goto loop1_not_tiny; }
	else {	// unlock any uninitialized frames (tiny infile)
		do {	(void) sem_post(nframes_avail);
			i = (i + 1u < framequeue_len ? i + 1u : 0);
		} while ( i != 0 );
	}
	do {	// wait for frame to finish encoding
		(void) sem_wait(&post_decoder[i]);

		// write tta to outfile
		dec_frame_write(
			&decbuf[i], &dstat, &user[i], outfile_fh,
			outfile_name, samplebytes, nchan, crc_read[i],
			dec_retval[i]
		);

		// mark frame as done and make available
		nbytes_tta_perframe[i] = 0;
		(void) sem_post(nframes_avail);
loop1_not_tiny:
		i = (i + 1u < framequeue_len ? i + 1u : 0);
	}
	while ( i != last );

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
	ichar         *const restrict dec_retval   =  frames->dec_retval;
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
		dec_retval[i] = (ichar) dec_frame_decode(
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
