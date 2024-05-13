//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// tta2enc_mt.c                                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//	deadlocks or aborts if (g_framequeue_len <= g_nthreads)             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <pthread.h>
#include <semaphore.h>

#include "../bits.h"
#include "../libttaR.h"
#include "../splint.h"

#include "bufs.h"
#include "cli.h"
#include "debug.h"
#include "formats.h"
#include "main.h"
#include "pqueue.h"
#include "tta2.h"

//////////////////////////////////////////////////////////////////////////////

// TODO own header w/ inlined funcs
struct FileStats_EncMT {
	uint			nchan;
	enum TTASampleBytes	samplebytes;
	size_t			nsamples_perframe;
	size_t			decpcm_size;
};

//==========================================================================//

struct MTPQueue {
	pthread_spinlock_t	lock;
	struct PQueue		q;
};

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
	size_t				*ni32_perframe;
	/*@dependent@*/
	struct EncBuf			*encbuf;
	/*@dependent@*/
	struct LibTTAr_CodecState_User	*user;
};

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

//--------------------------------------------------------------------------//

struct MTArg_EncIO {
	struct MTArg_EncIO_Frames	frames;
	struct MTArg_IO_Outfile		outfile;
	struct MTArg_IO_Infile 		infile;
	/*@temp@*/
	struct FileStats_EncMT		*fstat;
	/*@temp@*/
	struct SeekTable 		*seektable;
	/*@temp@*/
	struct EncStats			*estat_out;
};

struct MTArg_Encoder {
	struct MTArg_Encoder_Frames	frames;
	/*@temp@*/
	struct FileStats_EncMT		*fstat;
};

//////////////////////////////////////////////////////////////////////////////

/*@null@*/
#undef arg
static void *encmt_io(struct MTArg_EncIO *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		*arg->frames.post_encoder,
		*arg->frames.ni32_perframe,
		arg->outfile.outfile_fh,
		arg->infile.infile_fh,
		*arg->seektable,
		*arg->estat_out
@*/
;

//--------------------------------------------------------------------------//

#undef seektable
#undef estat_out
#undef outfile
static void encmt_frame_write(
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

//==========================================================================//

/*@null@*/
#undef arg
static void *encmt_encoder(struct MTArg_Encoder *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		arg->frames.queue,
		*arg->frames.post_encoder,
		*arg->encbuf
@*/
;

//--------------------------------------------------------------------------//

#undef priv
#undef user
#undef encbuf
static void encmt_frame_encode(
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	struct EncBuf *const restrict encbuf, size_t, enum TTASampleBytes,
	uint
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*priv,
		*user_out,
		encbuf->ttabuf_len,
		*encbuf->i32buf,
		encbuf->ttabuf,
		*encbuf->ttabuf
@*/
;

//==========================================================================//

// TODO put in own header
#undef fstat_c
static void fstatencmt_init(
	/*@out@*/ struct FileStats_EncMT *const restrict fstat_c,
	const struct FileStats *const restrict
)
/*@modifies	*fstat_c@*/
;

#undef io
#undef encoder
static void
encmtstate_init(
	/*@out@*/ struct MTArg_EncIO *const restrict io,
	/*@out@*/ struct MTArg_Encoder *const restrict encoder,
	uint, size_t, uint, enum TTASampleBytes, const FILE *const restrict,
	const char *const, const FILE *const restrict, const char *const,
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
		io->frames.navailable,
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
static void encmtstate_free(
	struct MTArg_EncIO *const restrict io,
	struct MTArg_Encoder *const restrict encoder, uint
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*io,
		io->frames.navailable,
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

//////////////////////////////////////////////////////////////////////////////

void
ttaenc_loop_mt(
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
	pthread_t *thread_encode = NULL;
	struct FileStats_EncMT fstat_c;
	const size_t samplebuf_len = fstat->buflen;
	const uint framequeue_len = FRAMEQUEUE_LEN(nthreads);
	uint i;
	union {	int d; } t;

	// init
	fstatencmt_init(&fstat_c, fstat);
	//
	encmtstate_init(
		&state_io, &state_encoder, framequeue_len, samplebuf_len,
		fstat_c.nchan, fstat_c.samplebytes, outfile, outfile_name,
		infile, infile_name, seektable, estat_out, &fstat_c
	);
	//
	thread_encode = calloc((size_t) nthreads, sizeof *thread_encode);
	assert(thread_encode != NULL);
	if UNLIKELY ( thread_encode == NULL ){
		error_sys(errno, "calloc", NULL);
	}

	// create
	for ( i = 0; i < nthreads; ++i ){
		t.d = pthread_create(
			&thread_encode[i], NULL,
			(void *(*)(void *)) encmt_encoder, &state_encoder
		);
		if UNLIKELY ( t.d != 0 ){
			error_sys(t.d, "pthread_create", NULL);
		}
	}

	// the main thread does the file-io
	(void) encmt_io(&state_io);

	// join
	for ( i = 0; i < nthreads; ++i ){
		t.d = pthread_join(thread_encode[i], NULL);
		if UNLIKELY ( t.d != 0 ){
			error_sys_nf(t.d, "pthread_join", NULL);
		}
	}

	// cleanup
	encmtstate_free(&state_io, &state_encoder, framequeue_len);
	free(thread_encode);

	return;
}

//==========================================================================//

/*@null@*/
static void *
encmt_io(struct MTArg_EncIO *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		*arg->frames.post_encoder,
		*arg->frames.ni32_perframe,
		arg->outfile.outfile_fh,
		arg->infile.infile_fh,
		*arg->seektable,
		*arg->estat_out
@*/
{
	struct MTArg_EncIO_Frames *const restrict frames  = &arg->frames;
	struct MTArg_IO_Outfile   *const restrict outfile = &arg->outfile;
	struct MTArg_IO_Infile    *const restrict infile  = &arg->infile;
	struct FileStats_EncMT *const restrict fstat      =  arg->fstat;
	struct SeekTable       *const restrict seektable  =  arg->seektable;
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

	struct EncStats estat;
	const size_t nbytes_pcm_perframe = (
		nsamples_perframe * nchan * samplebytes
	);
	size_t readlen = nbytes_pcm_perframe;
	size_t nmemb_read;
	size_t nsamples_flat_read_total = 0;
	bool start_writing = false, at_end = false, truncated = false;
	size_t framecnt = 0;
	uint i = 0, last;
	union {	uint u; } t;

	memset(&estat, 0x00, sizeof estat);
	goto loop0_entr;
	do {
		++framecnt;
		i = (i + 1u < framequeue_len ? i + 1u : 0);
		if ( (! start_writing) ){
			if ( i == 0 ){
				start_writing = true;
			}
			else {	goto io_read; }
		}

		// wait for frame to finish encoding
		(void) sem_wait(&post_encoder[i]);

		// write tta to outfile
		encmt_frame_write(
			&encbuf[i], seektable, &estat, &user[i], outfile_fh,
			outfile_name, nchan
		);
loop0_entr:
		if ( (! g_flag.quiet) && (framecnt % (size_t) 64 == 0) ){
			errprint_spinner();
		}
io_read:
		// read pcm from infile
		readlen		= enc_readlen(
			nsamples_perframe, nsamples_flat_read_total,
			decpcm_size, samplebytes, nchan
		);
		if ( readlen == 0 ){
			ni32_perframe[i] = 0;
			break;
		}
		nmemb_read	= fread(
			encbuf[i].pcmbuf, (size_t) samplebytes, readlen,
			infile_fh
		);
		ni32_perframe[i]          = nmemb_read;
		nsamples_flat_read_total += nmemb_read;
		//
		if UNLIKELY ( nmemb_read != readlen ){
			if UNLIKELY ( ferror(infile_fh) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile_name, framecnt
				);
			}
			at_end    = true;
			truncated = true;
		}
		// check for truncated sample
		t.u = (uint) (nmemb_read % nchan);
		if UNLIKELY ( t.u != 0 ){
			warning_tta("%s: frame %zu: last sample truncated, "
				"zero-padding", infile_name, framecnt
			);
			t.u = encmt_frame_zeropad(
				encbuf[i].pcmbuf, nmemb_read, t.u,
				samplebytes, nchan
			);
			ni32_perframe[i] += t.u;
			at_end    = true;
			truncated = true;
		}

		// make frame available
		(void) sem_post(nframes_avail);
	}
	while ( ! at_end );
	if ( truncated ){
		i = (i + 1u < framequeue_len ? i + 1u : 0);
	}
	last = i;

	// unlock any uninitialized frames (tiny infile)
	if ( ! start_writing ){
		do {
			(void) sem_post(nframes_avail);
			i = (i + 1u < framequeue_len ? i + 1u : 0);
		}
		while ( i != 0 );
		truncated = true;
	}

	// write the remaining frames
	if ( ! truncated ){
		goto loop1_entr;
	}
	do {
		// wait for frame to finish encoding
		(void) sem_wait(&post_encoder[i]);

		// write tta to outfile
		encmt_frame_write(
			&encbuf[i], seektable, &estat, &user[i], outfile_fh,
			outfile_name, nchan
		);

		// mark frame as done and make available
		ni32_perframe[i] = 0;
		(void) sem_post(nframes_avail);
loop1_entr:
		i = (i + 1u < framequeue_len ? i + 1u : 0);
	}
	while ( i != last );

	*arg->estat_out = estat;
	return NULL;
}

static void
encmt_frame_write(
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
		encbuf->ttabuf, user.nbytes_tta_total, (size_t) 1, outfile
	);
	if UNLIKELY ( t.z != (size_t) 1 ){
		error_sys_nf(errno, "fwrite", outfile_name);
	}

	// write frame footer (crc)
	user.crc = htole32(user.crc);
	t.z = fwrite(&user.crc, sizeof user.crc, (size_t) 1, outfile);
	if UNLIKELY ( t.z != (size_t) 1 ){
		error_sys_nf(errno, "fwrite", outfile_name);
	}
	user.nbytes_tta_total += sizeof user.crc;

	// update seektable
	seektable_add(
		seektable, user.nbytes_tta_total, estat.nframes, outfile_name
	);

	// update estat
	estat.nframes		+= (size_t) 1;
	estat.nsamples		+= user.ni32_total;
	estat.nsamples_perchan	+= (size_t) (user.ni32_total / nchan);
	estat.nbytes_encoded	+= user.nbytes_tta_total;

	*estat_out = estat;
	return;
}

//==========================================================================//

/*@null@*/
static void *
encmt_encoder(struct MTArg_Encoder *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		arg->frames.queue,
		*arg->frames.post_encoder,
		*arg->encbuf
@*/
{
	struct MTArg_Encoder_Frames *const restrict frames = &arg->frames;
	struct FileStats_EncMT      *const restrict fstat  =  arg->fstat;
	//
	sem_t         *const restrict nframes_avail  =  frames->navailable;
	struct MTPQueue *const restrict queue        = &frames->queue;
	sem_t         *const restrict post_encoder   =  frames->post_encoder;
	size_t        *const restrict ni32_perframe  =  frames->ni32_perframe;
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
		encmt_frame_encode(
			priv, &user[i], &encbuf[i], ni32_perframe[i],
			samplebytes, nchan
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

static void
encmt_frame_encode(
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	struct EncBuf *const restrict encbuf, size_t ni32_perframe,
	enum TTASampleBytes samplebytes, uint nchan
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*priv,
		*user_out,
		encbuf->ttabuf_len,
		*encbuf->i32buf,
		encbuf->ttabuf,
		*encbuf->ttabuf
@*/
{
	struct LibTTAr_CodecState_User user;
	size_t ni32_target = ni32_perframe;
	union {	size_t	z;
		int	d;
	} t;
#ifdef NDEBUG
	(void) t.d;	// gcc
#endif
	// convert pcm to i32
	t.z = libttaR_pcm_read(
		encbuf->i32buf, encbuf->pcmbuf, ni32_target,
		samplebytes
	);
	assert(t.z == ni32_target);

	// encode i32 to tta
	memset(&user, 0x00, sizeof user);
	user.is_new_frame  = true;
	user.ni32_perframe = ni32_perframe;
	goto loop_entr;
	do {
		encbuf_adjust(encbuf, g_samplebuf_len, nchan, samplebytes);
		ni32_target = ni32_perframe - user.ni32_total;
loop_entr:
		t.d = libttaR_tta_encode(
			&encbuf->ttabuf[user.nbytes_tta_total],
			&encbuf->i32buf[user.ni32_total],
			encbuf->ttabuf_len - user.nbytes_tta_total,
			encbuf->i32buf_len - user.ni32_total,
			ni32_target, priv, &user, samplebytes, nchan
		);
		assert(t.d == 0);
	}
	while ( ! user.frame_is_finished );

	*user_out = user;
	return;
}

//==========================================================================//

// TODO inline in own header
static void
fstatencmt_init(
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

static void
encmtstate_init(
	/*@out@*/ struct MTArg_EncIO *const restrict io,
	/*@out@*/ struct MTArg_Encoder *const restrict encoder,
	uint framequeue_len, size_t samplebuf_len, uint nchan,
	enum TTASampleBytes samplebytes,
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
		io->frames.navailable,
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
			&io->frames.encbuf[i], samplebuf_len, nchan,
			samplebytes
		);
		assert(t.z == samplebuf_len * nchan);
	}

	// io->outfile
	io->outfile.fh			= (FILE *) outfile;
	io->outfile.name		= outfile_name;

	// io->infile
	io->infile.fh			= (FILE *) infile;
	io->infile.name			= infile_name;

	// io other
	io->fstat			= (struct FileStats_EncMT *) fstat;
	io->seektable			= (struct SeekTable *) seektable;
	io->estat_out			= (struct EncStats *) estat_out;

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
	encoder->fstat			= (struct FileStats_EncMT *) fstat;

	return;
}

static void
encmtstate_free(
	struct MTArg_EncIO *const restrict io,
	struct MTArg_Encoder *const restrict encoder, uint framequeue_len
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*io,
		io->frames.navailable,
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
	union {	int d; } t;

	// io
	t.d = sem_destroy(io->frames.navailable);
	if ( t.d != 0 ){
		if UNLIKELY ( t.d != 0 ){
			error_sys_nf(t.d, "sem_destroy", NULL);
		}
	}
	free(io->frames.navailable);
	//
	t.d = sem_destroy(io->frames.post_encoder);
	if ( t.d != 0 ){
		if UNLIKELY ( t.d != 0 ){
			error_sys_nf(t.d, "sem_destroy", NULL);
		}
	}
	free(io->frames.post_encoder);
	//
	free(io->frames.ni32_perframe);
	//
	for ( i = 0; i < framequeue_len; ++i ){
		encbuf_free(&io->frames.encbuf[i]);
	}
	free(io->frames.encbuf);
	//
	free(io->frames.user);

	// encoder
	t.d = pthread_spin_destroy(&encoder->frames.queue.lock);
	if ( t.d != 0 ){
		if UNLIKELY ( t.d != 0 ){
			error_sys_nf(t.d, "pthread_spin_destroy", NULL);
		}
	}

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
