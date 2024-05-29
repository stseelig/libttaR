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
#undef nsamples_flat_2pad
static int dec_frame_decode(
	struct DecBuf *const restrict decbuf,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	enum TTASampleBytes, uint, size_t, size_t,
	/*@out@*/ size_t *const restrict nsamples_flat_2pad
)
/*@modifies	*decbuf->i32buf,
		*decbuf->pcmbuf,
		*priv,
		*user_out,
		*nsamples_flat_2pad
@*/
;

#undef decbuf
#undef dstat_out
#undef outfile
static void dec_frame_write(
	struct DecBuf *const restrict decbuf,
	/*@in@*/ struct DecStats *const restrict dstat_out,
	struct LibTTAr_CodecState_User *const restrict,
	const char *const restrict, FILE *const restrict outfile,
	const char *const restrict, enum TTASampleBytes, uint, u32, ichar,
	size_t, size_t
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*decbuf->pcmbuf,
		*dstat_out,
		outfile
@*/
;

static CONST size_t dec_ni32_perframe(size_t, size_t, size_t, uint) /*@*/;

#undef pcmbuf
static COLD void dec_frame_zeropad(
	u8 *const restrict pcmbuf, size_t, size_t, enum TTASampleBytes
)
/*@modifies	*pcmbuf@*/
;

//--------------------------------------------------------------------------//

#undef arg
/*@null@*/
static HOT void *decmt_io(struct MTArg_DecIO *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		*arg->frames.post_decoder,
		*arg->frames.ni32_perframe,
		*arg->frames.decbuf,
		*arg->frames.crc_read,
		arg->outfile.fh,
		arg->infile.fh,
		*arg->seektable,
		*arg->dstat_out
@*/
;

#undef arg
/*@null@*/
static HOT void *decmt_decoder(struct MTArg_Decoder *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		arg->frames.queue,
		*arg->frames.post_decoder,
		*arg->frames.decbuf,
		*arg->frames.dec_retval
@*/
;

//////////////////////////////////////////////////////////////////////////////

HOT void
decst_loop(
	const struct SeekTable *const restrict seektable,
	/*@out@*/ struct DecStats *const restrict dstat_out,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name
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
	const size_t              framelen     = fstat->framelen;
	const size_t              buflen       = fstat->buflen;
	const size_t              nsamples_enc = fstat->nsamples_enc;
	const uint                nchan        = (uint) fstat->nchan;
	const enum TTASampleBytes samplebytes  = fstat->samplebytes;

	struct LibTTAr_CodecState_Priv *priv = NULL;
	struct LibTTAr_CodecState_User user;
	struct DecBuf decbuf;
	struct DecStats dstat;
	//
	size_t ni32_perframe, nbytes_tta_perframe, framesize_tta, nbytes_read;
	size_t nsamples_perchan_dec_total = 0;
	size_t nframes_target = seektable->nmemb, nframes_read = 0;
	ichar dec_retval;
	size_t nsamples_flat_2pad;

	u32 crc_read;
	union {	size_t	z;
		int	d;
	} t;

	// setup buffers
	t.z = decbuf_init(&decbuf, buflen, SAMPLEBUF_LEN_DEFAULT, nchan);
	assert(t.z == (size_t) (buflen * nchan));
	//
	t.z = libttaR_codecstate_priv_size(nchan);
	assert(t.z != 0);
	priv = malloc(t.z);
	if UNLIKELY ( priv == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(priv != NULL);

	memset(&dstat, 0x00, sizeof dstat);
	goto loop_entr;
	do {
		// ENCFMT_TTA1
		// get size of tta-frame from seektable
		framesize_tta = (size_t) letoh32(
			seektable->table[nframes_read]
		);
		if ( framesize_tta <= (sizeof crc_read) ){
			warning_tta(
				"%s: frame %zu: malformed seektable entry",
				infile_name, nframes_read
			);
			break;
		}
		else {	framesize_tta -= (sizeof crc_read); }
		ni32_perframe = dec_ni32_perframe(
			nsamples_perchan_dec_total, nsamples_enc, framelen,
			nchan
		);
		// malformed seektable check
		if ( ni32_perframe == 0 ){ break; }
		nsamples_perchan_dec_total += ni32_perframe / nchan;

		// read tta from infile
		decbuf_check_adjust(&decbuf, framesize_tta, nchan);
		nbytes_read = fread(
			decbuf.ttabuf, (size_t) 1u, framesize_tta, infile
		);
		nbytes_tta_perframe = nbytes_read;
		//
		if UNLIKELY ( nbytes_read != framesize_tta ){
			if UNLIKELY ( ferror(infile) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile_name, nframes_read
				);
			}
			goto loop0_truncated;
		}
		// read frame footer (crc); kept as little-endian
		t.z = fread(&crc_read, sizeof crc_read, (size_t) 1u, infile);
		if UNLIKELY ( t.z != (size_t) 1u ){
			if UNLIKELY ( ferror(infile) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile_name, nframes_read
				);
			}
loop0_truncated:
			crc_read       = 0;
			nframes_target = 0;
		}
		++nframes_read;

		// decode frame
		dec_retval = dec_frame_decode(
			&decbuf, priv, &user, samplebytes, nchan,
			ni32_perframe, nbytes_tta_perframe,
			&nsamples_flat_2pad
		);

		// write pcm to outfile
		dec_frame_write(
			&decbuf, &dstat, &user, infile_name, outfile,
			outfile_name, samplebytes, nchan, crc_read,
			dec_retval, nsamples_flat_2pad, nbytes_tta_perframe
		);
loop_entr:
		if ( (! g_flag.quiet) && (nframes_read % SPINNER_FRQ == 0) ){
			errprint_spinner();
		}
	}
	while ( nframes_target-- != 0 );

	// cleanup
	free(priv);
	decbuf_free(&decbuf);

	*dstat_out = dstat;
	return;
}

// see encmt_loop in mode_encode_loop.c for a comment on the threads layout
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
	pthread_t thread_io;
	pthread_t *thread_decoder = NULL;
	struct FileStats_DecMT fstat_c;
	struct DecStats dstat;
	const size_t samplebuf_len = fstat->buflen;
	const uint framequeue_len = FRAMEQUEUE_LEN(nthreads);
	uint i;
	union {	int d; } t;

	assert(nthreads >= 1u);

	// init
	memset(&dstat, 0x00, sizeof dstat);
	decmt_fstat_init(&fstat_c, fstat);
	decmt_state_init(
		&state_io, &state_decoder, framequeue_len, samplebuf_len,
		outfile, outfile_name, infile, infile_name, seektable, &dstat,
		&fstat_c
	);
	//
	if ( nthreads > 1u ){
		thread_decoder = calloc(
			(size_t) (nthreads - 1u), sizeof *thread_decoder
		);
		if UNLIKELY ( thread_decoder == NULL ){
			error_sys(errno, "calloc", NULL);
		}
		assert(thread_decoder != NULL);
	}

	// create
	t.d = pthread_create(
		&thread_io, NULL, (void *(*)(void *)) decmt_io, &state_io
	);
	if UNLIKELY ( t.d != 0 ){
		error_sys(t.d, "pthread_create", NULL);
	}
	//
	for ( i = 0; i < nthreads - 1u; ++i ){
		t.d = pthread_create(
			&thread_decoder[i], NULL,
			(void *(*)(void *)) decmt_decoder, &state_decoder
		);
		if UNLIKELY ( t.d != 0 ){
			error_sys(t.d, "pthread_create", NULL);
		}
	}
	//
	(void) decmt_decoder(&state_decoder);

	// join
	for ( i = 0; i < nthreads - 1u; ++i ){
		(void) pthread_join(thread_decoder[i], NULL);
	}
	//
	(void) pthread_join(thread_io, NULL);

	// cleanup
	decmt_state_free(&state_io, &state_decoder, framequeue_len);
	if ( nthreads > 1u ){
		free(thread_decoder);
	}

	*dstat_out = dstat;
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
	size_t nbytes_tta_perframe,
	/*@out@*/ size_t *const restrict nsamples_flat_2pad
)
/*@modifies	*decbuf->i32buf,
		*decbuf->pcmbuf,
		*priv,
		*user_out,
		*nsamples_flat_2pad
@*/
{
	int r;
	struct LibTTAr_CodecState_User user = LIBTTAr_CODECSTATE_USER_INIT;
	size_t pad_target = 0;
	union {	size_t z; } t;
#ifdef NDEBUG
	(void) t.z;	// gcc
#endif

	goto loop_entr;
	do {
		// frame has a truncated last sample; needs zero-padding later
		pad_target     = (size_t) (nchan - (ni32_perframe % nchan));
		ni32_perframe += pad_target;
loop_entr:
		// decode tta to i32
		r = libttaR_tta_decode(
			decbuf->i32buf, decbuf->ttabuf, decbuf->i32buf_len,
			decbuf->ttabuf_len, ni32_perframe,
			nbytes_tta_perframe, priv, &user, samplebytes, nchan,
			ni32_perframe, nbytes_tta_perframe
		);

		// with the way the decoding is setup, RET_AGAIN shouldn't
		//   happen
		// >=RET_INVAL may happen if the last sample is truncated
		assert((r == LIBTTAr_RET_DONE)
		      ||
		       (r == LIBTTAr_RET_DECFAIL)
		      ||
		       (r >= LIBTTAr_RET_INVAL)
		);
	}
	while ( r >= LIBTTAr_RET_INVAL );

	if ( r == LIBTTAr_RET_DECFAIL ){
		pad_target     += ni32_perframe - user.ni32_total;
		user.ni32_total = ni32_perframe;
	}

	// convert i32 to pcm
	t.z = libttaR_pcm_write(
		decbuf->pcmbuf, decbuf->i32buf, user.ni32_total, samplebytes
	);
	assert(t.z == user.ni32);

	*user_out           = user;
	*nsamples_flat_2pad = pad_target;
	return r;
}

static void
dec_frame_write(
	struct DecBuf *const restrict decbuf,
	/*@in@*/ struct DecStats *const restrict dstat_out,
	struct LibTTAr_CodecState_User *const restrict user_in,
	const char *const restrict infile_name,
	FILE *const restrict outfile, const char *const restrict outfile_name,
	enum TTASampleBytes samplebytes, uint nchan, u32 crc_read,
	ichar dec_retval, size_t nsamples_flat_2pad,
	size_t nbytes_tta_perframe
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
	u32 crc_read_h = letoh32(crc_read);
	union { uint	u;
		size_t	z;
	} t;

	// DECFAIL or >=INVAL
	if UNLIKELY ( dec_retval != (ichar) LIBTTAr_RET_DONE ){
		if ( dec_retval == (ichar) LIBTTAr_RET_DECFAIL ){
			warning_tta("%s: frame %zu: decoding failed",
				infile_name, dstat.nframes
			);
		}
		else {	// dec_retval >= (ichar) LIBTTAr_RET_INVAL
			warning_tta("%s: frame %zu: last sample truncated, "
				"zero-padding", infile_name, dstat.nframes
			);
		}
		// zero-pad
		dec_frame_zeropad(
			decbuf->pcmbuf, user.ni32_total, nsamples_flat_2pad,
			samplebytes
		);
		// re-calc crc
		user.crc = libttaR_crc32(decbuf->ttabuf, nbytes_tta_perframe);
	}

	// check frame crc
	if UNLIKELY ( user.crc != crc_read_h ){
		warning_tta("%s: frame %zu is corrupted; bad crc",
			infile_name, dstat.nframes
		);
	}
	user.nbytes_tta_total += sizeof crc_read_h;

	// write frame
	t.z = fwrite(decbuf->pcmbuf, samplebytes, user.ni32_total, outfile);
	if UNLIKELY ( t.z != user.ni32 ){
		error_sys(errno, "fwrite", outfile_name);
	}

	// update dstat
	dstat.nframes          += (size_t) 1u;
	dstat.nsamples_flat    += user.ni32_total;
	dstat.nsamples_perchan += (size_t) (user.ni32_total / nchan);
	dstat.nbytes_decoded   += user.nbytes_tta_total;

	*dstat_out = dstat;
	return;
}

// returns ni32_perframe
static CONST size_t
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

static COLD void
dec_frame_zeropad(
	u8 *const restrict pcmbuf, size_t pcmbuf_nsamples_flat,
	size_t nsamples_flat_2pad, enum TTASampleBytes samplebytes
)
/*@modifies	*pcmbuf@*/
{
	const size_t ind     = (size_t) (
		(pcmbuf_nsamples_flat - nsamples_flat_2pad) * samplebytes
	);
	const size_t padsize = (size_t) (nsamples_flat_2pad * samplebytes);
	memset(&pcmbuf[ind], 0x00, padsize);
	return;
}

//==========================================================================//

/*@null@*/
static HOT void *
decmt_io(struct MTArg_DecIO *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		*arg->frames.post_decoder,
		*arg->frames.ni32_perframe,
		*arg->frames.decbuf,
		*arg->frames.crc_read,
		arg->outfile.fh,
		arg->infile.fh,
		*arg->seektable,
		*arg->dstat_out
@*/
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
	size_t        *const restrict nbytes_tta_perframe  = (
		frames->nbytes_tta_perframe
	);
	struct DecBuf *const restrict decbuf             = frames->decbuf;
	u32           *const restrict crc_read           = frames->crc_read;
	struct LibTTAr_CodecState_User *const restrict user = frames->user;
	ichar         *const restrict dec_retval         = frames->dec_retval;
	size_t        *const restrict nsamples_flat_2pad = (
		frames->nsamples_flat_2pad
	);
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
	const size_t nsamples_enc             = fstat->nsamples_enc;

	struct DecStats dstat;
	size_t framesize_tta, nbytes_read;
	size_t nsamples_perchan_dec_total = 0;
	bool start_writing = false;
	size_t nframes_target = seektable->nmemb, nframes_read = 0;
	uint i = 0, last;
	union {	uint	u;
		size_t	z;
	} t;

	goto loop0_entr;
	do {
		// ENCFMT_TTA1
		// get size of tta-frame from seektable
		framesize_tta = (size_t) letoh32(
			seektable->table[nframes_read]
		);
		if ( framesize_tta <= (sizeof *crc_read) ){
			warning_tta(
				"%s: frame %zu: malformed seektable entry",
				infile_name, nframes_read
			);
			nbytes_tta_perframe[i] = 0;
			break;
		}
		else {	framesize_tta -= (sizeof *crc_read); }
		ni32_perframe[i] = dec_ni32_perframe(
			nsamples_perchan_dec_total, nsamples_enc,
			nsamples_perframe, nchan
		);
		// malformed seektable check
		if ( ni32_perframe[i] == 0 ){ break; }
		nsamples_perchan_dec_total += ni32_perframe[i] / nchan;

		// read tta from infile
		decbuf_check_adjust(&decbuf[i], framesize_tta, nchan);
		nbytes_read = fread(
			decbuf[i].ttabuf, (size_t) 1u, framesize_tta,
			infile_fh
		);
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
			crc_read[i]    = 0;
			nframes_target = 0;
		}
		++nframes_read;

		// make frame available
		(void) sem_post(nframes_avail);

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
			&decbuf[i], &dstat, &user[i], infile_name, outfile_fh,
			outfile_name, samplebytes, nchan, crc_read[i],
			dec_retval[i], nsamples_flat_2pad[i],
			nbytes_tta_perframe[i]
		);
loop0_entr:
		if ( (! g_flag.quiet) && (nframes_read % SPINNER_FRQ == 0) ){
			errprint_spinner();
		}
loop0_read:
		;
	}
	while ( nframes_target-- != 0 );
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
			&decbuf[i], &dstat, &user[i], infile_name, outfile_fh,
			outfile_name, samplebytes, nchan,
			crc_read[i], dec_retval[i], nsamples_flat_2pad[i],
			nbytes_tta_perframe[i]
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
static HOT void *
decmt_decoder(struct MTArg_Decoder *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		arg->frames.queue,
		*arg->frames.post_decoder,
		*arg->frames.decbuf,
		*arg->frames.dec_retval
@*/
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
	struct DecBuf *const restrict decbuf             = frames->decbuf;
	struct LibTTAr_CodecState_User *const restrict user = frames->user;
	ichar         *const restrict dec_retval         = frames->dec_retval;
	size_t        *const restrict nsamples_flat_2pad = (
		frames->nsamples_flat_2pad
	);
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
			ni32_perframe[i], nbytes_tta_perframe[i],
			&nsamples_flat_2pad[i]
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
