//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mode_encode_loop.c                                                 //
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

#undef priv
#undef user
#undef encbuf
static void enc_frame_encode(
	struct EncBuf *const restrict encbuf,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	enum TTASampleBytes, uint, size_t
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		encbuf->ttabuf_len,
		*encbuf->i32buf,
		encbuf->ttabuf,
		*encbuf->ttabuf,
		*priv,
		*user_out
@*/
;

#undef seektable
#undef estat_out
#undef outfile
static void enc_frame_write(
	struct EncBuf *const restrict,
	struct SeekTable *const restrict seektable,
	/*@in@*/ struct EncStats *const restrict estat_out,
	struct LibTTAr_CodecState_User *const restrict,
	FILE *const restrict outfile, const char *const restrict, uint
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*seektable,
		*estat_out,
		outfile
@*/
;

static CONST size_t enc_readlen(
	size_t, size_t, size_t, enum TTASampleBytes, uint
)
/*@*/
;

#undef pcmbuf
static COLD uint enc_frame_zeropad(
	u8 *const restrict pcmbuf, size_t, uint, enum TTASampleBytes, uint
)
/*@modifies	*pcmbuf@*/
;

//--------------------------------------------------------------------------//

#undef arg
/*@null@*/
static HOT void *encmt_io(struct MTArg_EncIO *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		*arg->frames.post_encoder,
		*arg->frames.ni32_perframe,
		*arg->frames.encbuf,
		arg->outfile.fh,
		arg->infile.fh,
		*arg->seektable,
		*arg->estat_out
@*/
;

#undef arg
/*@null@*/
static HOT void *encmt_encoder(struct MTArg_Encoder *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		arg->frames.queue,
		*arg->frames.post_encoder,
		*arg->frames.encbuf
@*/
;

//////////////////////////////////////////////////////////////////////////////

HOT void
encst_loop(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct EncStats *const restrict estat_out,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name
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
{
	const size_t              decpcm_size       = fstat->decpcm_size;
	const size_t              nsamples_perframe = fstat->framelen;
	const size_t              buflen            = fstat->buflen;
	const uint                nchan             = (uint) fstat->nchan;
	const enum TTASampleBytes samplebytes       = fstat->samplebytes;

	struct LibTTAr_CodecState_Priv *priv = NULL;
	struct LibTTAr_CodecState_User user;
	struct EncBuf encbuf;
	struct EncStats estat;
	//
	size_t readlen, nmemb_read;
	size_t ni32_perframe, nsamples_flat_read_total = 0;
	size_t nframes_read = 0;
	union { uint	u;
		size_t	z;
	} t;

	// setup buffers
	t.z = encbuf_init(
		&encbuf, buflen, TTABUF_LEN_DEFAULT, nchan, samplebytes
	);
	assert(t.z == (size_t) (buflen * nchan));
	//
	t.z = libttaR_codecstate_priv_size(nchan);
	assert(t.z != 0);
	priv = malloc(t.z);
	if UNLIKELY ( priv == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(priv != NULL);

	memset(&estat, 0x00, sizeof estat);
	goto loop_entr;
	do {
		nmemb_read = fread(
			encbuf.pcmbuf, (size_t) samplebytes, readlen,
			infile
		);
		ni32_perframe             = nmemb_read;
		nsamples_flat_read_total += nmemb_read;
		//
		if UNLIKELY ( nmemb_read != readlen ){
			// ??? not sure if the filechecks allow us to be here
			if UNLIKELY ( ferror(infile) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile, nframes_read
				);
			}
			// forces readlen to 0
			nsamples_flat_read_total = SIZE_MAX;
		}

		// check for truncated sample
		t.u = (uint) (nmemb_read % nchan);
		if UNLIKELY ( t.u != 0 ){
			warning_tta("%s: frame %zu: last sample truncated, "
				"zero-padding", infile_name, nframes_read
			);
			t.u = enc_frame_zeropad(
				encbuf.pcmbuf, nmemb_read, t.u, samplebytes,
				nchan
			);
			ni32_perframe += t.u;
		}

		// encode frame
		enc_frame_encode(
			&encbuf, priv, &user, samplebytes, nchan,
			ni32_perframe
		);

		// write frame
		enc_frame_write(
			&encbuf, seektable, &estat, &user, outfile,
			outfile_name, nchan
		);

		++nframes_read;
loop_entr:
		if ( (! g_flag.quiet) && (nframes_read % SPINNER_FRQ == 0) ){
			errprint_spinner();
		}

		// read pcm from infile
		readlen	= enc_readlen(
			nsamples_perframe, nsamples_flat_read_total,
			decpcm_size, samplebytes, nchan
		);
	}
	while (	readlen != 0 );

	// cleanup
	free(priv);
	encbuf_free(&encbuf);

	*estat_out = estat;
	return;
}

// threads layout:
//
//	- the io thread is created first, so it can get a head start on
//  filling up the framequeue
//
//      - then (nthreads - 1u) coder threads are created
//
//      - the main thread then becomes the last coder thread
//
//      - after the main thread finishes coding, the other coder threads are
//  joined, and then finally, the io thread is joined
void
encmt_loop(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct EncStats *const restrict estat_out,
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
		*seektable,
		*estat_out,
		outfile,
		infile
@*/
{
	struct MTArg_EncIO state_io;
	struct MTArg_Encoder state_encoder;
	pthread_t thread_io;
	pthread_t *thread_encoder = NULL;
	struct FileStats_EncMT fstat_c;
	struct EncStats estat;
	const size_t samplebuf_len = fstat->buflen;
	const uint framequeue_len = FRAMEQUEUE_LEN(nthreads);
	uint i;
	union {	int d; } t;

	assert(nthreads >= 1u);

	// init
	memset(&estat, 0x00, sizeof estat);
	encmt_fstat_init(&fstat_c, fstat);
	encmt_state_init(
		&state_io, &state_encoder, framequeue_len, samplebuf_len,
		outfile, outfile_name, infile, infile_name, seektable, &estat,
		&fstat_c
	);
	//
	if ( nthreads > 1u ){
		thread_encoder = calloc(
			(size_t) (nthreads - 1u), sizeof *thread_encoder
		);
		if UNLIKELY ( thread_encoder == NULL ){
			error_sys(errno, "calloc", NULL);
		}
		assert(thread_encoder != NULL);
	}

	// create
	t.d = pthread_create(
		&thread_io, NULL, (void *(*)(void *)) encmt_io, &state_io
	);
	if UNLIKELY ( t.d != 0 ){
		error_sys(t.d, "pthread_create", NULL);
	}
	//
	for ( i = 0; i < nthreads - 1u; ++i ){
		t.d = pthread_create(
			&thread_encoder[i], NULL,
			(void *(*)(void *)) encmt_encoder, &state_encoder
		);
		if UNLIKELY ( t.d != 0 ){
			error_sys(t.d, "pthread_create", NULL);
		}
	}
	//
	(void) encmt_encoder(&state_encoder);

	// join
	for ( i = 0; i < nthreads - 1u; ++i ){
		(void) pthread_join(thread_encoder[i], NULL);
	}
	//
	(void) pthread_join(thread_io, NULL);

	// cleanup
	encmt_state_free(&state_io, &state_encoder, framequeue_len);
	if ( nthreads > 1u ){
		free(thread_encoder);
	}

	*estat_out = estat;
	return;
}

//==========================================================================//

static void
enc_frame_encode(
	struct EncBuf *const restrict encbuf,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	enum TTASampleBytes samplebytes, uint nchan, size_t ni32_perframe
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		encbuf->ttabuf_len,
		*encbuf->i32buf,
		encbuf->ttabuf,
		*encbuf->ttabuf,
		*priv,
		*user_out
@*/
{
	struct LibTTAr_CodecState_User user = LIBTTAr_CODECSTATE_USER_INIT;
	size_t ni32_target = ni32_perframe;
	union {	size_t	z;
		int	d;
	} t;
#ifdef NDEBUG
	(void) t.d;	// gcc
#endif
	// convert pcm to i32
	t.z = libttaR_pcm_read(
		encbuf->i32buf, encbuf->pcmbuf, ni32_target, samplebytes
	);
	assert(t.z == ni32_target);

	// encode i32 to tta
	goto loop_entr;
	do {
		encbuf_adjust(encbuf, TTABUF_LEN_DEFAULT);
		ni32_target = ni32_perframe - user.ni32_total;
loop_entr:
		t.d = libttaR_tta_encode(
			&encbuf->ttabuf[user.nbytes_tta_total],
			&encbuf->i32buf[user.ni32_total],
			encbuf->ttabuf_len - user.nbytes_tta_total,
			encbuf->i32buf_len - user.ni32_total,
			ni32_target, priv, &user, samplebytes, nchan,
			ni32_perframe
		);
		assert((t.d == LIBTTAr_RET_DONE)
		      ||
		       (t.d == LIBTTAr_RET_AGAIN)
		);
	}
	while ( t.d == LIBTTAr_RET_AGAIN );

	*user_out = user;
	return;
}

static void
enc_frame_write(
	struct EncBuf *const restrict encbuf,
	struct SeekTable *const restrict seektable,
	/*@in@*/ struct EncStats *const restrict estat_out,
	struct LibTTAr_CodecState_User *const restrict user_in,
	FILE *const restrict outfile, const char *const restrict outfile_name,
	uint nchan
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*seektable,
		*estat_out,
		outfile
@*/
{
	struct LibTTAr_CodecState_User user = *user_in;
	struct EncStats estat = *estat_out;
	union {	size_t z; } t;

	// write frame
	t.z = fwrite(
		encbuf->ttabuf, user.nbytes_tta_total, (size_t) 1u, outfile
	);
	if UNLIKELY ( t.z != (size_t) 1u ){
		error_sys(errno, "fwrite", outfile_name);
	}

	// write frame footer (crc)
	user.crc = htole32(user.crc);
	t.z = fwrite(&user.crc, sizeof user.crc, (size_t) 1u, outfile);
	if UNLIKELY ( t.z != (size_t) 1u ){
		error_sys(errno, "fwrite", outfile_name);
	}
	user.nbytes_tta_total += sizeof user.crc;

	// update seektable
	seektable_add(
		seektable, user.nbytes_tta_total, estat.nframes, outfile_name
	);

	// update estat
	estat.nframes          += (size_t) 1u;
	estat.nsamples_flat    += user.ni32_total;
	estat.nsamples_perchan += (size_t) (user.ni32_total / nchan);
	estat.nbytes_encoded   += user.nbytes_tta_total;

	*estat_out = estat;
	return;
}

// returns nmemb for fread
static CONST size_t
enc_readlen(
	size_t nsamples_perframe, size_t nsamples_flat_read,
	size_t decpcm_size, enum TTASampleBytes samplebytes, uint nchan
)
/*@*/
{
	size_t r = nsamples_perframe * nchan;
	size_t new_total;
	union { size_t z; } t;

	// truncated short circuit
	if ( nsamples_flat_read == SIZE_MAX ){
		return 0;
	}

	t.z = (size_t) ((nsamples_flat_read + r) * samplebytes);
	if ( t.z > decpcm_size ){
		new_total = (size_t) (nsamples_flat_read * samplebytes);
		if ( new_total < decpcm_size ){
			r  = decpcm_size - new_total;
			r /= (size_t) samplebytes;
		}
		else {	r = 0; }
	}
	return r;
}

// returns nmemb of buf zero-padded
static COLD uint
enc_frame_zeropad(
	u8 *const restrict pcmbuf, size_t nmemb_read, uint diff,
	enum TTASampleBytes samplebytes, uint nchan
)
/*@modifies	*pcmbuf@*/
{
	const uint   r   = nchan - diff;
	const size_t ind = (size_t) (nmemb_read * samplebytes);
	memset(&pcmbuf[ind], 0x00, (size_t) (r * samplebytes));
	return r;
}

//==========================================================================//

/*@null@*/
static HOT void *
encmt_io(struct MTArg_EncIO *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		*arg->frames.post_encoder,
		*arg->frames.ni32_perframe,
		*arg->frames.encbuf,
		arg->outfile.fh,
		arg->infile.fh,
		*arg->seektable,
		*arg->estat_out
@*/
{
	struct MTArg_EncIO_Frames *const restrict  frames  = &arg->frames;
	struct MTArg_IO_File      *const restrict  outfile = &arg->outfile;
	struct MTArg_IO_File      *const restrict  infile  = &arg->infile;
	const struct FileStats_EncMT *const restrict fstat =  arg->fstat;
	struct SeekTable *const restrict         seektable =  arg->seektable;
	//
	sem_t          *const restrict nframes_avail =  frames->navailable;
	sem_t          *const restrict post_encoder  =  frames->post_encoder;
	size_t         *const restrict ni32_perframe =  frames->ni32_perframe;
	struct EncBuf  *const restrict encbuf        =  frames->encbuf;
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
	const size_t decpcm_size              = fstat->decpcm_size;

	struct EncStats estat = *arg->estat_out;
	size_t readlen, nmemb_read;
	size_t nsamples_flat_read_total = 0;
	bool start_writing = false;
	size_t nframes_read = 0;
	uint i = 0, last;
	union {	uint u; } t;

	goto loop0_entr;
	do {
		// read pcm from infile
		nmemb_read = fread(
			encbuf[i].pcmbuf, (size_t) samplebytes, readlen,
			infile_fh
		);
		ni32_perframe[i]          = nmemb_read;
		nsamples_flat_read_total += nmemb_read;
		//
		if UNLIKELY ( nmemb_read != readlen ){
			// ??? not sure if the filechecks allow us to be here
			if UNLIKELY ( ferror(infile_fh) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile_name, nframes_read
				);
			}
			// forces readlen to 0
			nsamples_flat_read_total = SIZE_MAX;
		}

		// check for truncated sample
		t.u = (uint) (nmemb_read % nchan);
		if UNLIKELY ( t.u != 0 ){
			warning_tta("%s: frame %zu: last sample truncated, "
				"zero-padding", infile_name, nframes_read
			);
			t.u = enc_frame_zeropad(
				encbuf[i].pcmbuf, nmemb_read, t.u,
				samplebytes, nchan
			);
			ni32_perframe[i] += t.u;
		}

		// make frame available
		(void) sem_post(nframes_avail);

		++nframes_read;
		i = (i + 1u < framequeue_len ? i + 1u : 0);
		if ( ! start_writing ){
			if ( i == 0 ){
				start_writing = true;
			}
			else {	goto loop0_read; }
		}

		// wait for frame to finish encoding
		(void) sem_wait(&post_encoder[i]);

		// write tta to outfile
		enc_frame_write(
			&encbuf[i], seektable, &estat, &user[i], outfile_fh,
			outfile_name, nchan
		);
loop0_entr:
		if ( (! g_flag.quiet) && (nframes_read % SPINNER_FRQ == 0) ){
			errprint_spinner();
		}
loop0_read:
		// calc size of pcm to read from infile
		readlen = enc_readlen(
			nsamples_perframe, nsamples_flat_read_total,
			decpcm_size, samplebytes, nchan
		);
	}
	while ( readlen != 0 );
	ni32_perframe[i] = 0;
	last = i;

	// write the remaining frames
	if ( start_writing ){ goto loop1_not_tiny; }
	else {	// unlock any uninitialized frames (tiny infile)
		do {	(void) sem_post(nframes_avail);
			i = (i + 1u < framequeue_len ? i + 1u : 0);
		} while ( i != 0 );
	}
	do {	// wait for frame to finish encoding
		(void) sem_wait(&post_encoder[i]);

		// write tta to outfile
		enc_frame_write(
			&encbuf[i], seektable, &estat, &user[i], outfile_fh,
			outfile_name, nchan
		);

		// mark frame as done and make available
		ni32_perframe[i] = 0;
		(void) sem_post(nframes_avail);
loop1_not_tiny:
		i = (i + 1u < framequeue_len ? i + 1u : 0);
	}
	while ( i != last );

	*arg->estat_out = estat;
	return NULL;
}

/*@null@*/
static HOT void *
encmt_encoder(struct MTArg_Encoder *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		arg->frames.queue,
		*arg->frames.post_encoder,
		*arg->frames.encbuf
@*/
{
	struct MTArg_Encoder_Frames  *const restrict frames = &arg->frames;
	const struct FileStats_EncMT *const restrict fstat  =  arg->fstat;
	//
	sem_t         *const restrict nframes_avail  =  frames->navailable;
	struct MTPQueue *const restrict queue        = &frames->queue;
	sem_t         *const restrict post_encoder   =  frames->post_encoder;
	const size_t  *const restrict ni32_perframe  =  frames->ni32_perframe;
	struct EncBuf *const restrict encbuf         =  frames->encbuf;
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
		// encode frame
		enc_frame_encode(
			&encbuf[i], priv, &user[i], samplebytes, nchan,
			ni32_perframe[i]
		);

		// unlock frame
		(void) sem_post(&post_encoder[i]);
loop_entr:
		// wait for a frame to be available
		(void) sem_wait(nframes_avail);

		// get frame id from encode queue
		(void) pthread_spin_lock(&queue->lock);
		i = pdequeue(&queue->q);
		(void) pthread_spin_unlock(&queue->lock);
	}
	while ( ni32_perframe[i] != 0 );

	// cleanup
	free(priv);

	return NULL;
}

// EOF ///////////////////////////////////////////////////////////////////////
