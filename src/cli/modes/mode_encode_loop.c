//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mode_encode_loop.c                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef S_SPLINT_S
#include "../../splint.h"
#endif

/* ------------------------------------------------------------------------ */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	// memset

#include "../../bits.h"
#include "../../libttaR.h"

#include "../alloc.h"
#include "../cli.h"
#include "../debug.h"
#include "../formats.h"
#include "../main.h"

#include "bufs.h"
#include "mt-struct.h"
#include "pqueue.h"
#include "threads.h"

//////////////////////////////////////////////////////////////////////////////

#undef priv
#undef user
#undef encbuf
static void enc_frame_encode(
	struct EncBuf *restrict encbuf,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *restrict user_out,
	enum LibTTAr_SampleBytes, uint, size_t
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
	struct EncBuf *restrict, struct SeekTable *restrict seektable,
	/*@in@*/ struct EncStats *restrict estat_out,
	struct LibTTAr_CodecState_User *restrict,
	FILE *restrict outfile, const char *restrict, uint
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
	size_t, size_t, size_t, enum LibTTAr_SampleBytes, uint
)
/*@*/
;

#undef pcmbuf
static NOINLINE COLD uint enc_frame_zeropad(
	u8 *restrict pcmbuf, size_t, uint, enum LibTTAr_SampleBytes, uint
)
/*@modifies	*pcmbuf@*/
;

//--------------------------------------------------------------------------//

#undef arg
START_ROUTINE_ABI
static HOT start_routine_ret encmt_io(struct MTArg_EncIO *restrict arg)
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
START_ROUTINE_ABI
static start_routine_ret encmt_encoder_wrapper(void *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg
@*/
;

#undef arg
static HOT int encmt_encoder(struct MTArg_Encoder *restrict arg)
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

/**@fn encst_loop
 * @brief the single-threaded encoder
 *
 * @param seektable[in] the seektable struct
 * @param estat_out[out] the encode stats return struct
 * @param fstat[in] the bloated file stats struct
 * @param outfile[in] the destination file
 * @param outfile_name[in] the name of the destination file (warnings/errors)
 * @param infile[in] the source file
 * @param infile_name[in] the name of the source file (warnings/errors)
**/
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
	const size_t decpcm_size       = fstat->decpcm_size;
	const size_t nsamples_perframe = fstat->framelen;
	const size_t buflen            = fstat->buflen;
	const uint   nchan             = (uint) fstat->nchan;
	const enum LibTTAr_SampleBytes samplebytes = fstat->samplebytes;

	struct LibTTAr_CodecState_Priv *priv = NULL;
	struct LibTTAr_CodecState_User user;
	struct EncBuf encbuf;
	struct EncStats estat;
	//
	size_t readlen, nmemb_read;
	size_t ni32_perframe, nsamples_flat_read_total = 0;
	size_t nframes_read = 0;
	union {	size_t	z; } result;
	union { uint	u; } tmp;

	// setup
	encbuf_init(
		&encbuf, buflen, TTABUF_LEN_DEFAULT, nchan, samplebytes,
		CBM_SINGLE_THREADED
	);
	result.z = libttaR_codecstate_priv_size(nchan);
	assert(result.z != 0);
	priv     = malloc_check(result.z);

	memset(&estat, 0x00, sizeof estat);
	goto loop_entr;
	do {
		nmemb_read = fread(
			encbuf.pcmbuf, (size_t) samplebytes, readlen, infile
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
					infile_name, nframes_read
				);
			}
			// forces readlen to 0
			nsamples_flat_read_total = SIZE_MAX;
		}

		// check for truncated sample
		tmp.u = (uint) (nmemb_read % nchan);
		if UNLIKELY ( tmp.u != 0 ){
			warning_tta("%s: frame %zu: last sample truncated, "
				"zero-padding", infile_name, nframes_read
			);
			tmp.u = enc_frame_zeropad(
				encbuf.pcmbuf, nmemb_read, tmp.u, samplebytes,
				nchan
			);
			ni32_perframe += tmp.u;
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

		nframes_read += 1u;
loop_entr:
		if ( (! g_flag.quiet) && (nframes_read % SPINNER_FREQ == 0) ){
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
	codecbuf_free(&encbuf, CBM_SINGLE_THREADED);

	*estat_out = estat;
	return;
}

/**@fn encmt_loop
 * @brief the multi-threaded encodeer handler
 *
 * @param seektable[in] the seektable struct
 * @param estat_out[out] the encode stats return struct
 * @param fstat[in] the bloated file stats struct
 * @param outfile[in] the destination file
 * @param outfile_name[in] the name of the destination file (warnings/errors)
 * @param infile[in] the source file
 * @param infile_name[in] the name of the source file (warnings/errors)
 * @param nthreads the number of encoder threads to use
 *
 * @note threads layout:
 *     - the io thread is created first, so it can get a head start on filling
 *   up the framequeue
 *     - then (nthreads - 1u) coder threads are created
 *     - the main thread then becomes the last coder thread
 *     - when a created coder thread is finished, it detaches itself
 *     - after the main thread finishes coding, the io thread is joined
**/
void
encmt_loop(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct EncStats *const restrict estat_out,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name,
	const uint nthreads
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
	struct MTArg_EncIO io_state;
	struct MTArg_Encoder encoder_state;
	thread_p io_thread, encoder_thread;
	struct FileStats_EncMT fstat_c;
	struct EncStats estat;
	const size_t samplebuf_len = fstat->buflen;
	const uint framequeue_len = FRAMEQUEUE_LEN(nthreads);
	uint i;

	assert(nthreads > 0);

	// setup/init
	memset(&estat, 0x00, sizeof estat);
	encmt_fstat_init(&fstat_c, fstat);
	encmt_state_init(
		&io_state, &encoder_state, framequeue_len, samplebuf_len,
		outfile, outfile_name, infile, infile_name, seektable, &estat,
		&fstat_c
	);

	// create coders
	thread_create(
		&io_thread,
		(START_ROUTINE_ABI start_routine_ret (*)(void *)) encmt_io,
		&io_state
	);
	for ( i = 0; i < nthreads - 1u; ++i ){
		thread_create(
			&encoder_thread, encmt_encoder_wrapper, &encoder_state
		);
	}
	(void) encmt_encoder(&encoder_state);

	// wait for i/o
	thread_join(&io_thread);

	// cleanup
	encmt_state_free(&io_state, &encoder_state, framequeue_len);

	*estat_out = estat;
	return;
}

//==========================================================================//

/**@fn dec_frame_encode
 * @brief encode a TTA frame
 *
 * @param encbuf[in out] the encode buffers struct
 * @param priv[out] the private state struct
 * @param user_out[out] the user state return struct
 * @param samplebytes number of bytes per PCM sample
 * @param nchan number of audio channels
 * @param ni32_perframe total number of i32 in a TTA frame
 *
 * @return what libttaR_tta_decode returned
**/
static void
enc_frame_encode(
	struct EncBuf *const restrict encbuf,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	const enum LibTTAr_SampleBytes samplebytes, const uint nchan,
	const size_t ni32_perframe
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
	enum LibTTAr_EncRetVal status;
	struct LibTTAr_CodecState_User user = LIBTTAr_CODECSTATE_USER_INIT;
	struct LibTTAr_EncMisc misc;
	UNUSED union {	size_t z; } result;

	assert(encbuf->i32buf != NULL);

	// convert pcm to i32
	result.z = libttaR_pcm_read(
		encbuf->i32buf, encbuf->pcmbuf, ni32_perframe, samplebytes
	);
	assert(result.z != 0);

	// encode i32 to tta
	misc.ni32_perframe = ni32_perframe;
	misc.samplebytes   = samplebytes;
	misc.nchan         = nchan;
	goto loop_entr;
	do {
		encbuf_adjust(encbuf, TTABUF_LEN_DEFAULT, nchan);
loop_entr:
		misc.dest_len    = encbuf->ttabuf_len - user.nbytes_tta_total;
		misc.src_len     = encbuf->i32buf_len - user.ni32_total;
		misc.ni32_target = ni32_perframe - user.ni32_total;

		status = libttaR_tta_encode(
			&encbuf->ttabuf[user.nbytes_tta_total],
			&encbuf->i32buf[user.ni32_total],
			priv, &user, &misc
		);
		assert((status == LIBTTAr_ERV_DONE)
		      ||
		       (status == LIBTTAr_ERV_AGAIN)
		);
	}
	while ( status == LIBTTAr_ERV_AGAIN );

	*user_out = user;
	return;
}

/**@fn enc_frame_write
 * @brief write a TTA frame
 *
 * @param encbuf[in out] the encode buffers struct
 * @param estat_out[out] the encode stats return struct
 * @param user_in[in] the user state struct
 * @param infile_name[in] the name of the source file (warnings/errors)
 * @param outfile[in] the destination file
 * @param outfile_name[in] the name of the destination file (warnings/errors)
 * @param samplebytes number of bytes per PCM sample
 * @param nchan number of audio channels
**/
static void
enc_frame_write(
	struct EncBuf *const restrict encbuf,
	struct SeekTable *const restrict seektable,
	/*@in@*/ struct EncStats *const restrict estat_out,
	struct LibTTAr_CodecState_User *const restrict user_in,
	FILE *const restrict outfile, const char *const restrict outfile_name,
	const uint nchan
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
	union {	size_t z; } result;

	// write frame
	result.z = fwrite(
		encbuf->ttabuf, user.nbytes_tta_total, (size_t) 1u, outfile
	);
	if UNLIKELY ( result.z != (size_t) 1u ){
		error_sys(errno, "fwrite", outfile_name);
	}

	// write frame footer (crc)
	user.crc = htole32(user.crc);
	result.z = fwrite(&user.crc, sizeof user.crc, (size_t) 1u, outfile);
	if UNLIKELY ( result.z != (size_t) 1u ){
		error_sys(errno, "fwrite", outfile_name);
	}
	user.nbytes_tta_total += sizeof user.crc;

	// update seektable
	seektable_add(seektable, user.nbytes_tta_total, outfile_name);

	// update estat
	estat.nframes          += 1u;
	estat.nsamples_flat    += user.ni32_total;
	estat.nsamples_perchan += (size_t) (user.ni32_total / nchan);
	estat.nbytes_encoded   += user.nbytes_tta_total;

	*estat_out = estat;
	return;
}

/**@fn enc_readlen
 * @brief calculates nmemb for fread for the next frame
 *
 * @param nsamples_perframe number of samples of 'nchan' channels per frame
 * @param nsamples_flat_read total number of audio samples read
 * @param decpcm_size total size of the decoded PCM in the source file
 * @param samplebytes number of bytes per PCM sample
 * @param nchan number of audio channels
 *
 * @return nmemb for fread
**/
static CONST size_t
enc_readlen(
	const size_t nsamples_perframe, const size_t nsamples_flat_read,
	const size_t decpcm_size, const enum LibTTAr_SampleBytes samplebytes,
	const uint nchan
)
/*@*/
{
	size_t retval = nsamples_perframe * nchan;
	size_t new_total;
	union { size_t z; } tmp;

	// truncated short circuit
	if ( nsamples_flat_read == SIZE_MAX ){
		return 0;
	}

	tmp.z = (size_t) ((nsamples_flat_read + retval) * samplebytes);
	if ( tmp.z > decpcm_size ){
		new_total = (size_t) (nsamples_flat_read * samplebytes);
		if ( new_total < decpcm_size ){
			retval  = decpcm_size - new_total;
			retval /= (size_t) samplebytes;
		}
		else {	retval  = 0; }
	}
	return retval;
}

/**@fn enc_frame_zeropad
 * @brief zero-pads the PCM buffer (truncated last sample)
 *
 * @param pcmbuf[out] the PCM buffer
 * @param nmemb_read total number of samples read into the PCM buffer
 * @param diff number of channels in the last sample
 * @param samplebytes number of bytes per PCM sample
 * @param nchan number of audio channels
 *
 * @return the total number of samples in the PCM buffer zero-padded
**/
static NOINLINE COLD uint
enc_frame_zeropad(
	u8 *const restrict pcmbuf, const size_t nmemb_read, const uint diff,
	enum LibTTAr_SampleBytes samplebytes, const uint nchan
)
/*@modifies	*pcmbuf@*/
{
	const uint   retval = nchan - diff;
	const size_t idx    = (size_t) (nmemb_read * samplebytes);
	memset(&pcmbuf[idx], 0x00, (size_t) (retval * samplebytes));
	return retval;
}

//==========================================================================//

/**@fn encmt_io
 * @brief the I/O thread function for the multi-threaded encoder
 *
 * @param arg state for the thread
 *
 * @pre arg->frames->nmemb > the number of encoder threads
 *
 * @retval (start_routine_ret) 0
**/
START_ROUTINE_ABI
static HOT start_routine_ret
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
	semaphore_p    *const restrict nframes_avail =  frames->navailable;
	semaphore_p    *const restrict post_encoder  =  frames->post_encoder;
	size_t         *const restrict ni32_perframe =  frames->ni32_perframe;
	struct EncBuf  *const restrict encbuf        =  frames->encbuf;
	struct LibTTAr_CodecState_User *const restrict user = frames->user;
	//
	FILE       *const restrict outfile_fh   = outfile->fh;
	const char *const restrict outfile_name = outfile->name;
	FILE       *const restrict infile_fh    = infile->fh;
	const char *const restrict infile_name  = infile->name;
	//
	const uint framequeue_len                  = frames->nmemb;
	const uint nchan                           = fstat->nchan;
	const enum LibTTAr_SampleBytes samplebytes = fstat->samplebytes;
	const size_t nsamples_perframe             = fstat->nsamples_perframe;
	const size_t decpcm_size                   = fstat->decpcm_size;

	struct EncStats estat = *arg->estat_out;
	size_t readlen, nmemb_read;
	size_t nsamples_flat_read_total = 0;
	bool start_writing = false;
	size_t nframes_read = 0;
	uint i = 0, last;
	union {	uint u; } tmp;

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
		tmp.u = (uint) (nmemb_read % nchan);
		if UNLIKELY ( tmp.u != 0 ){
			warning_tta("%s: frame %zu: last sample truncated, "
				"zero-padding", infile_name, nframes_read
			);
			tmp.u = enc_frame_zeropad(
				encbuf[i].pcmbuf, nmemb_read, tmp.u,
				samplebytes, nchan
			);
			ni32_perframe[i] += tmp.u;
		}

		// make frame available
		semaphore_post(nframes_avail);

		nframes_read += 1u;
		i = pqueue_next(i, framequeue_len);
		// all frame pcmbuf's are filled before writing out any TTA
		if ( ! start_writing ){
			if ( i != 0 ){
				goto loop0_read;
			}
			start_writing = true;
		}

		// wait for frame to finish encoding
		semaphore_wait(&post_encoder[i]);

		// write tta to outfile
		enc_frame_write(
			&encbuf[i], seektable, &estat, &user[i], outfile_fh,
			outfile_name, nchan
		);
loop0_entr:
		if ( (! g_flag.quiet) && (nframes_read % SPINNER_FREQ == 0) ){
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
	if ( start_writing ){
		goto loop1_not_tiny;
	}
	else {	// unlock any uninitialized frames (tiny infile)
		do {	semaphore_post(nframes_avail);
			i = pqueue_next(i, framequeue_len);
		} while ( i != 0 );
	}
	do {	// wait for frame to finish encoding
		semaphore_wait(&post_encoder[i]);

		// write tta to outfile
		enc_frame_write(
			&encbuf[i], seektable, &estat, &user[i], outfile_fh,
			outfile_name, nchan
		);

		// mark frame as done and make available
		ni32_perframe[i] = 0;
		semaphore_post(nframes_avail);
loop1_not_tiny:
		i = pqueue_next(i, framequeue_len);
	}
	while ( i != last );

	*arg->estat_out = estat;
	return (start_routine_ret) 0;
}

/**@fn encmt_encoder_wrapper
 * @brief wraps the mt-encoder function. the thread detaches itself before
 *   returning
 *
 * @param arg state for the thread
 *
 * @return (start_routine_ret) 0
**/
START_ROUTINE_ABI
static start_routine_ret
encmt_encoder_wrapper(void *const restrict arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg
@*/
{
	(void) encmt_encoder(arg);
	thread_detach_self();
	return (start_routine_ret) 0;
}

/**@fn encmt_encoder
 * @brief the encoder thread function for the multi-threaded encoder
 *
 * @param arg state for the thread
 *
 * @return 0
**/
static HOT int
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
	struct MTPQueue *const restrict queue        = &frames->queue;
	semaphore_p   *const restrict nframes_avail  =  frames->navailable;
	semaphore_p   *const restrict post_encoder   =  frames->post_encoder;
	const size_t  *const restrict ni32_perframe  =  frames->ni32_perframe;
	struct EncBuf *const restrict encbuf         =  frames->encbuf;
	struct LibTTAr_CodecState_User *const restrict user = frames->user;
	//
	const uint nchan                           = fstat->nchan;
	const enum LibTTAr_SampleBytes samplebytes = fstat->samplebytes;

	i32 *i32buf = NULL;
	struct LibTTAr_CodecState_Priv *priv = NULL;
	uint i;
	union {	size_t z; } result;

	// setup
	i32buf   = calloc_check(encbuf[0].i32buf_len, sizeof *i32buf);
	//
	result.z = libttaR_codecstate_priv_size(nchan);
	assert(result.z != 0);
	priv     = malloc_check(result.z);

	goto loop_entr;
	do {
		// encode frame
		encbuf[i].i32buf = i32buf;
		enc_frame_encode(
			&encbuf[i], priv, &user[i], samplebytes, nchan,
			ni32_perframe[i]
		);

		// unlock frame
		semaphore_post(&post_encoder[i]);
loop_entr:
		// wait for a frame to be available
		semaphore_wait(nframes_avail);

		// get frame id from encode queue
		spinlock_lock(&queue->lock);
		{
			i = pqueue_pop(&queue->q);
		}
		spinlock_unlock(&queue->lock);
	}
	while ( ni32_perframe[i] != 0 );

	// cleanup
	free(i32buf);
	free(priv);

	return 0;
}

// EOF ///////////////////////////////////////////////////////////////////////
