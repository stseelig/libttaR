/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mode_encode_loop.c                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../libttaR.h"

#include "../alloc.h"
#include "../byteswap.h"
#include "../cli.h"
#include "../common.h"
#include "../debug.h"
#include "../formats.h"
#include "../main.h"

#include "./bufs.h"
#include "./mt-struct.h"
#include "./pqueue.h"
#include "./threads.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef priv
#undef user
#undef encbuf
static NOINLINE enum LibTTAr_EncRetVal enc_frame_encode(
	struct EncBuf *RESTRICT encbuf,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *RESTRICT priv,
	/*@out@*/ struct LibTTAr_CodecState_User *RESTRICT user_out,
	enum LibTTAr_SampleBytes, unsigned int, size_t
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
static NOINLINE void enc_frame_write(
	struct EncBuf *RESTRICT, struct SeekTable *RESTRICT seektable,
	/*@in@*/ struct EncStats *RESTRICT estat_out,
	struct LibTTAr_CodecState_User *RESTRICT, const char *RESTRICT,
	FILE *RESTRICT outfile, const char *RESTRICT, unsigned int, int8_t
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

CONST
static size_t enc_readlen(
	size_t, size_t, size_t, enum LibTTAr_SampleBytes, unsigned int
)
/*@*/
;

#undef pcmbuf
COLD
static NOINLINE unsigned int enc_frame_zeropad(
	uint8_t *RESTRICT pcmbuf, size_t, unsigned int,
	enum LibTTAr_SampleBytes, unsigned int
)
/*@modifies	*pcmbuf@*/
;

/* ------------------------------------------------------------------------ */

#undef arg
HOT
START_ROUTINE_ABI
static start_routine_ret encmt_io(struct MTArg_EncIO *RESTRICT arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		*arg->frames.post_encoder,
		*arg->frames.ni32_perframe,
		*arg->frames.encbuf,
		arg->outfile.handle,
		arg->infile.handle,
		*arg->seektable,
		*arg->estat_out
@*/
;

#undef arg
START_ROUTINE_ABI
static start_routine_ret encmt_encoder_wrapper(void *const RESTRICT arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg
@*/
;

#undef arg
HOT
static start_routine_ret encmt_encoder(struct MTArg_Encoder *RESTRICT arg)
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

/* //////////////////////////////////////////////////////////////////////// */

/**@fn encst_loop
 * @brief the single-threaded encoder
 *
 * @param seektable    - seektable struct
 * @param estat_out    - encode stats return struct
 * @param fstat        - bloated file stats struct
 * @param outfile      - destination file
 * @param outfile_name - name of the destination file (warnings/errors)
 * @param infile       - source file
 * @param infile_name  - name of the source file (warnings/errors)
**/
HOT
BUILD NOINLINE void
encst_loop(
	struct SeekTable *const RESTRICT seektable,
	/*@out@*/ struct EncStats *const RESTRICT estat_out,
	const struct FileStats *const RESTRICT fstat,
	FILE *const RESTRICT outfile, const char *const outfile_name,
	FILE *const RESTRICT infile, const char *const infile_name
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
	const unsigned int nchan       = (unsigned int) fstat->nchan;
	const enum LibTTAr_SampleBytes samplebytes =    fstat->samplebytes;
	/* * */
	struct LibTTAr_CodecState_Priv *priv = NULL;
	struct LibTTAr_CodecState_User user;
	struct EncBuf encbuf;
	struct EncStats estat;
	/* * */
	size_t readlen, nmemb_read;
	size_t ni32_perframe, nsamples_flat_read_total = 0;
	size_t nframes_read = 0;
	int8_t enc_retval;
	union { unsigned int u; } tmp;

	/* setup */
	memset(&estat, 0x00, sizeof estat);
	encbuf_init(
		&encbuf, buflen, TTABUF_LEN_DEFAULT, nchan, samplebytes,
		CBM_SINGLE_THREADED
	);
	priv = priv_alloc(nchan);

	goto loop_entr;
	do {
		nmemb_read = fread(
			encbuf.pcmbuf, (size_t) samplebytes, readlen, infile
		);
		ni32_perframe             = nmemb_read;
		nsamples_flat_read_total += nmemb_read;

		if UNLIKELY ( nmemb_read != readlen ){
			/* ???: not sure if the filechecks let us be here */
			if UNLIKELY ( ferror(infile) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile_name, nframes_read
				);
			}
			/* forces readlen to 0 */
			nsamples_flat_read_total = SIZE_MAX;
		}

		/* check for truncated sample */
		tmp.u = (unsigned int) (nmemb_read % nchan);
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

		/* encode frame */
		enc_retval = (int8_t) enc_frame_encode(
			&encbuf, priv, &user, samplebytes, nchan,
			ni32_perframe
		);

		/* write frame */
		enc_frame_write(
			&encbuf, seektable, &estat, &user, infile_name,
			outfile, outfile_name, nchan, enc_retval
		);

		nframes_read += 1u;
loop_entr:
		if ( (! g_flag.quiet) && (nframes_read % SPINNER_FREQ == 0) ){
			errprint_spinner();
		}

		/* read PCM from infile */
		readlen	= enc_readlen(
			nsamples_perframe, nsamples_flat_read_total,
			decpcm_size, samplebytes, nchan
		);
	}
	while (	readlen != 0 );

	/* cleanup */
	priv_free(priv);
	codecbuf_free(&encbuf, CBM_SINGLE_THREADED);

	*estat_out = estat;
	return;
}

/**@fn encmt_loop
 * @brief the multi-threaded encodeer handler
 *
 * @param seektable    - seektable struct
 * @param estat_out    - encode stats return struct
 * @param fstat        - bloated file stats struct
 * @param outfile      - destination file
 * @param outfile_name - name of the destination file (warnings/errors)
 * @param infile       - source file
 * @param infile_name  - name of the source file (warnings/errors)
 * @param nthreads     - number of encoder threads to use
 *
 * @note threads layout:
 *     - the io thread is created first, so it can get a head start on filling
 *   up the framequeue
 *     - then (nthreads - 1u) coder threads are created
 *     - the main thread then becomes the last coder thread
 *     - when a created coder thread is finished, it detaches itself
 *     - after the main thread finishes coding, the io thread is joined
**/
BUILD NOINLINE void
encmt_loop(
	struct SeekTable *const RESTRICT seektable,
	/*@out@*/ struct EncStats *const RESTRICT estat_out,
	const struct FileStats *const RESTRICT fstat,
	FILE *const RESTRICT outfile, const char *const outfile_name,
	FILE *const RESTRICT infile, const char *const infile_name,
	const unsigned int nthreads
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
	const size_t        samplebuf_len = fstat->buflen;
	const unsigned int framequeue_len = FRAMEQUEUE_LEN(nthreads);
	unsigned int i;

	assert(nthreads > 0);

	/* setup/init */
	memset(&estat, 0x00, sizeof estat);
	encmt_fstat_init(&fstat_c, fstat);
	encmt_state_init(
		&io_state, &encoder_state, framequeue_len, samplebuf_len,
		outfile, outfile_name, infile, infile_name, seektable, &estat,
		&fstat_c
	);

	/* create coders */
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

	/* wait for i/o */
	thread_join(&io_thread);

	/* cleanup */
	encmt_state_free(&io_state, &encoder_state, framequeue_len);

	*estat_out = estat;
	return;
}

/* ======================================================================== */

/**@fn enc_frame_encode
 * @brief encode a TTA frame
 *
 * @param encbuf        - encode buffers struct
 * @param priv          - private state struct
 * @param user_out      - user state return struct
 * @param samplebytes   - number of bytes per PCM sample
 * @param nchan         - number of audio channels
 * @param ni32_perframe - total number of i32 in a TTA frame
 *
 * @return what libttaR_tta_encode(3) returned
**/
static NOINLINE enum LibTTAr_EncRetVal
enc_frame_encode(
	struct EncBuf *const RESTRICT encbuf,
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const RESTRICT priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const RESTRICT user_out,
	const enum LibTTAr_SampleBytes samplebytes, const unsigned int nchan,
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

	/* convert PCM to I32 */
	result.z = libttaR_pcm_read(
		encbuf->i32buf, encbuf->pcmbuf, ni32_perframe, samplebytes
	);
	assert(result.z != 0);

	/* encode I32 to TTA */
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
		assert((status == LIBTTAr_ERV_OK_DONE)
		      ||
		       (status == LIBTTAr_ERV_OK_AGAIN)
		);
	}
	while ( status == LIBTTAr_ERV_OK_AGAIN );

	*user_out = user;
	return status;
}

/**@fn enc_frame_write
 * @brief write a TTA frame
 *
 * @param encbuf       - encode buffers struct
 * @param seektable    - seektable struct
 * @param estat_out    - encode stats return struct
 * @param user_in      - user state struct
 * @param infile_name  - name of the source file (warnings/errors)
 * @param outfile      - destination file
 * @param outfile_name - name of the destination file (warnings/errors)
 * @param nchan        - number of audio channels
 * @param enc_retval   - return value from enc_frame_encode()
**/
static NOINLINE void
enc_frame_write(
	struct EncBuf *const RESTRICT encbuf,
	struct SeekTable *const RESTRICT seektable,
	/*@in@*/ struct EncStats *const RESTRICT estat_out,
	struct LibTTAr_CodecState_User *const RESTRICT user_in,
	const char *const RESTRICT infile_name,
	FILE *const RESTRICT outfile, const char *const RESTRICT outfile_name,
	const unsigned int nchan, const int8_t enc_retval
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

	/* failure check */
	if UNLIKELY ( enc_retval != (int8_t) LIBTTAr_ERV_OK_DONE ){
		error_tta("%s: frame %zu: unexpected encode status (%"
			PRId8 ")",
			infile_name, estat.nframes, enc_retval
		);
	}

	/* write frame */
	result.z = fwrite(
		encbuf->ttabuf, user.nbytes_tta_total, SIZE_C(1), outfile
	);
	if UNLIKELY ( result.z != SIZE_C(1) ){
		error_sys(errno, "fwrite", outfile_name);
	}

	/* write frame footer (CRC) */
	user.crc = byteswap_htole_u32(user.crc);
	result.z = fwrite(&user.crc, sizeof user.crc, SIZE_C(1), outfile);
	if UNLIKELY ( result.z != SIZE_C(1) ){
		error_sys(errno, "fwrite", outfile_name);
	}
	user.nbytes_tta_total += sizeof user.crc;

	/* update seektable */
	seektable_add(seektable, user.nbytes_tta_total, outfile_name);

	/* update estat */
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
 * @param nsamples_perframe  - number of samples of 'nchan' channels per frame
 * @param nsamples_flat_read - total number of audio samples read
 * @param decpcm_size  - total size of the decoded PCM in the source file
 * @param samplebytes  - number of bytes per PCM sample
 * @param nchan number - of audio channels
 *
 * @return nmemb for fread
**/
CONST
static size_t
enc_readlen(
	const size_t nsamples_perframe, const size_t nsamples_flat_read,
	const size_t decpcm_size, const enum LibTTAr_SampleBytes samplebytes,
	const unsigned int nchan
)
/*@*/
{
	size_t retval = nsamples_perframe * nchan;
	size_t new_total;
	union { size_t z; } tmp;

	/* truncated short circuit */
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
 * @param pcmbuf      - PCM buffer
 * @param nmemb_read  - total number of samples read into the PCM buffer
 * @param diff        - number of channels in the last sample
 * @param samplebytes - number of bytes per PCM sample
 * @param nchan       - number of audio channels
 *
 * @return total number of samples in the PCM buffer zero-padded
**/
COLD
static NOINLINE unsigned int
enc_frame_zeropad(
	uint8_t *const RESTRICT pcmbuf, const size_t nmemb_read,
	const unsigned int diff, enum LibTTAr_SampleBytes samplebytes,
	const unsigned int nchan
)
/*@modifies	*pcmbuf@*/
{
	const unsigned int retval = nchan - diff;
	const size_t       idx    = (size_t) (nmemb_read * samplebytes);

	memset(&pcmbuf[idx], 0x00, (size_t) (retval * samplebytes));

	return retval;
}

/* ======================================================================== */

/**@fn encmt_io
 * @brief the I/O thread function for the multi-threaded encoder
 *
 * @param arg - state for the thread
 *
 * @pre arg->frames->nmemb > the number of encoder threads
 *
 * @retval (start_routine_ret) 0
**/
HOT
START_ROUTINE_ABI
static start_routine_ret
encmt_io(struct MTArg_EncIO *const RESTRICT arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg->frames.navailable,
		*arg->frames.post_encoder,
		*arg->frames.ni32_perframe,
		*arg->frames.encbuf,
		arg->outfile.handle,
		arg->infile.handle,
		*arg->seektable,
		*arg->estat_out
@*/
{
	struct MTArg_EncIO_Frames *const RESTRICT  frames  = &arg->frames;
	struct MTArg_IO_File      *const RESTRICT  outfile = &arg->outfile;
	struct MTArg_IO_File      *const RESTRICT  infile  = &arg->infile;
	const struct FileStats_EncMT *const RESTRICT fstat =  arg->fstat;
	struct SeekTable *const RESTRICT         seektable =  arg->seektable;
	/* * */
	semaphore_p   *const RESTRICT nframes_avail =  frames->navailable;
	semaphore_p   *const RESTRICT post_encoder  =  frames->post_encoder;
	size_t        *const RESTRICT ni32_perframe =  frames->ni32_perframe;
	struct EncBuf *const RESTRICT encbuf        =  frames->encbuf;
	struct LibTTAr_CodecState_User *const RESTRICT user = frames->user;
	int8_t        *const RESTRICT enc_retval    = frames->enc_retval;
	/* * */
	FILE       *const RESTRICT outfile_handle   = outfile->handle;
	const char *const RESTRICT outfile_name     = outfile->name;
	FILE       *const RESTRICT infile_handle    = infile->handle;
	const char *const RESTRICT infile_name      = infile->name;
	/* * */
	const unsigned int framequeue_len          = frames->nmemb;
	const unsigned int nchan                   = fstat->nchan;
	const enum LibTTAr_SampleBytes samplebytes = fstat->samplebytes;
	const size_t nsamples_perframe             = fstat->nsamples_perframe;
	const size_t decpcm_size                   = fstat->decpcm_size;
	/* * */
	struct EncStats estat = *arg->estat_out;
	size_t readlen, nmemb_read;
	size_t nsamples_flat_read_total = 0;
	bool   start_writing  = false;
	size_t nframes_read   = 0;
	unsigned int idx = 0, last;
	union {	unsigned int u; } tmp;

	goto loop0_entr;
	do {
		/* read pcm from infile */
		nmemb_read = fread(
			encbuf[idx].pcmbuf, (size_t) samplebytes, readlen,
			infile_handle
		);
		ni32_perframe[idx]        = nmemb_read;
		nsamples_flat_read_total += nmemb_read;
		if UNLIKELY ( nmemb_read != readlen ){
			/* ???: not sure if the filechecks let us be here */
			if UNLIKELY ( ferror(infile_handle) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile_name, nframes_read
				);
			}
			/* forces readlen to 0 */
			nsamples_flat_read_total = SIZE_MAX;
		}

		/* check for truncated sample */
		tmp.u = (unsigned int) (nmemb_read % nchan);
		if UNLIKELY ( tmp.u != 0 ){
			warning_tta("%s: frame %zu: last sample truncated, "
				"zero-padding", infile_name, nframes_read
			);
			tmp.u = enc_frame_zeropad(
				encbuf[idx].pcmbuf, nmemb_read, tmp.u,
				samplebytes, nchan
			);
			ni32_perframe[idx] += tmp.u;
		}

		/* make frame available */
		semaphore_post(nframes_avail);

		nframes_read += 1u;
		idx = pqueue_next(idx, framequeue_len);
		/* all frame pcmbuf's are filled before writing out any TTA */
		if ( ! start_writing ){
			if ( idx != 0 ){
				goto loop0_read;
			}
			start_writing = true;
		}

		/* wait for frame to finish encoding */
		semaphore_wait(&post_encoder[idx]);

		/* write tta to outfile */
		enc_frame_write(
			&encbuf[idx], seektable, &estat, &user[idx],
			infile_name, outfile_handle, outfile_name, nchan,
			enc_retval[idx]
		);
loop0_entr:
		if ( (! g_flag.quiet) && (nframes_read % SPINNER_FREQ == 0) ){
			errprint_spinner();
		}
loop0_read:
		/* calc size of pcm to read from infile */
		readlen = enc_readlen(
			nsamples_perframe, nsamples_flat_read_total,
			decpcm_size, samplebytes, nchan
		);
	}
	while ( readlen != 0 );
	ni32_perframe[idx] = 0;
	last = idx;

	/* write the remaining frames */
	if ( start_writing ){
		goto loop1_not_tiny;
	}
	else {	/* unlock any uninitialized frames (tiny infile) */
		do {	semaphore_post(nframes_avail);
			idx = pqueue_next(idx, framequeue_len);
		}
		while ( idx != 0 );
	}
	do {	/* wait for frame to finish encoding */
		semaphore_wait(&post_encoder[idx]);

		/* write tta to outfile */
		enc_frame_write(
			&encbuf[idx], seektable, &estat, &user[idx],
			infile_name, outfile_handle, outfile_name, nchan,
			enc_retval[idx]
		);

		/* mark frame as done and make available */
		ni32_perframe[idx] = 0;
		semaphore_post(nframes_avail);
loop1_not_tiny:
		idx = pqueue_next(idx, framequeue_len);
	}
	while ( idx != last );

	*arg->estat_out = estat;
	return (start_routine_ret) 0;
}

/**@fn encmt_encoder_wrapper
 * @brief wraps the mt-encoder function. the thread detaches itself before
 *   returning
 *
 * @param arg - state for the thread
 *
 * @return whatever encmt_encoder() returns
**/
START_ROUTINE_ABI
static start_routine_ret
encmt_encoder_wrapper(void *const RESTRICT arg)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*arg
@*/
{
	start_routine_ret retval;

	retval = encmt_encoder(arg);

	thread_detach_self();

	return retval;
}

/**@fn encmt_encoder
 * @brief the encoder thread function for the multi-threaded encoder
 *
 * @param arg - state for the thread
 *
 * @return (start_routine_ret) 0
**/
HOT
static start_routine_ret
encmt_encoder(struct MTArg_Encoder *const RESTRICT arg)
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
	struct MTArg_Encoder_Frames  *const RESTRICT frames = &arg->frames;
	const struct FileStats_EncMT *const RESTRICT fstat  =  arg->fstat;
	/* * */
	struct MTPQueue *const RESTRICT queue        = &frames->queue;
	semaphore_p    *const RESTRICT nframes_avail =  frames->navailable;
	semaphore_p    *const RESTRICT post_encoder  =  frames->post_encoder;
	const size_t   *const RESTRICT ni32_perframe =  frames->ni32_perframe;
	struct EncBuf  *const RESTRICT encbuf        =  frames->encbuf;
	struct LibTTAr_CodecState_User *const RESTRICT user = frames->user;
	int8_t         *const RESTRICT enc_retval    = frames->enc_retval;
	/* * */
	const unsigned int nchan                   = fstat->nchan;
	const enum LibTTAr_SampleBytes samplebytes = fstat->samplebytes;
	/* * */
	int32_t *i32buf = NULL;
	struct LibTTAr_CodecState_Priv *priv = NULL;
	unsigned int idx;

	/* setup */
	i32buf = calloc_check(encbuf[0].i32buf_len, sizeof *i32buf);
	priv   = priv_alloc(nchan);

	goto loop_entr;
	do {
		/* encode frame */
		encbuf[idx].i32buf = i32buf;
		enc_retval[idx]    = (int8_t) enc_frame_encode(
			&encbuf[idx], priv, &user[idx], samplebytes, nchan,
			ni32_perframe[idx]
		);

		/* unlock frame */
		semaphore_post(&post_encoder[idx]);
loop_entr:
		/* wait for a frame to be available */
		semaphore_wait(nframes_avail);

		/* get frame id from encode queue */
		spinlock_lock(&queue->lock);
		{
			idx = pqueue_pop(&queue->q);
		}
		spinlock_unlock(&queue->lock);
	}
	while ( ni32_perframe[idx] != 0 );

	/* cleanup */
	priv_free(priv);
	free(i32buf);

	return (start_routine_ret) 0;
}

/* EOF //////////////////////////////////////////////////////////////////// */
