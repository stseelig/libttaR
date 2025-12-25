/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mt-struct.c                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdlib.h>
#include <stdint.h>

#include "../../libttaR.h"

#include "../alloc.h"
#include "../common.h"
#include "../debug.h"
#include "../formats.h"

#include "./align.h"
#include "./mt-struct.h"
#include "./pqueue.h"
#include "./threads.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef io
static void encmt_state_init_allocs(
	/*@out@*/ struct MTArg_EncIO *RESTRICT io, size_t
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		io->frames.navailable,
		io->frames.post_encoder,
		io->frames.ni32_perframe,
		io->frames.encbuf,
		io->frames.user
@*/
/*@allocates	io->frames.navailable@*/
;

#undef io
static void decmt_state_init_allocs(
	/*@out@*/ struct MTArg_DecIO *RESTRICT io, size_t
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		io->frames.navailable,
		io->frames.post_decoder,
		io->frames.ni32_perframe,
		io->frames.nbytes_tta_perframe,
		io->frames.decbuf,
		io->frames.user,
		io->frames.nsamples_flat_2pad,
		io->frames.dec_retval,
		io->frames.crc_read
@*/
/*@allocates	io->frames.navailable@*/
;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn encmt_state_init
 * @brief initializes the multi-threaded encoder state structs
 *
 * @param io             - state struct for the io thread
 * @param encoder        - state struct for the encoder threads
 * @param framequeue_len - length of the framequeue
 * @param i32buf_len     - length of the encbuf->i32buf
 * @param outfile        - destination file
 * @param outfile_name   - name of the destination file (warnings/errors)
 * @param infile         - source file
 * @param infile_name    - name of the source file (warnings/errors)
 * @param seektable      - TTA seektable struct
 * @param estat_out      - encode stats return struct
 * @param fstat          - compacted file stats struct
**/
BUILD void
encmt_state_init(
	/*@out@*/ struct MTArg_EncIO *const RESTRICT io,
	/*@out@*/ struct MTArg_Encoder *const RESTRICT encoder,
	const unsigned int framequeue_len, const size_t i32buf_len,
	FILE *const RESTRICT outfile, const char *const outfile_name,
	FILE *const RESTRICT infile, const char *const infile_name,
	const struct SeekTable *const RESTRICT seektable,
	const struct EncStats *const RESTRICT estat_out,
	const struct FileStats_EncMT *const RESTRICT fstat
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
		io->frames.encbuf[].pcmbuf,
		io->frames.encbuf[].ttabuf
@*/
{
	unsigned int i;

	/* base allocations */
	encmt_state_init_allocs(io, (size_t) framequeue_len);

	/* io->frames */
	io->frames.nmemb	= framequeue_len;
	/* * */
	semaphore_init(io->frames.navailable, 0);
	/* * */
	for ( i = 0; i < framequeue_len; ++i ){
		semaphore_init(&io->frames.post_encoder[i], 0);
	}
	for ( i = 0; i < framequeue_len; ++i ){
		encbuf_init(
			&io->frames.encbuf[i], i32buf_len,
			TTABUF_LEN_DEFAULT, fstat->nchan, fstat->samplebytes,
			CBM_MULTI_THREADED
		);
	}

	/* io->outfile */
	io->outfile.handle	= outfile;
	io->outfile.name	= outfile_name;

	/* io->infile */
	io->infile.handle	= infile;
	io->infile.name		= infile_name;

	/* io other */
	io->fstat		= fstat;
	io->seektable		= (struct SeekTable *) seektable;
	io->estat_out		= (struct EncStats *) estat_out;

	/* encoder->frames */
	spinlock_init(&encoder->frames.queue.lock);
	pqueue_init(&encoder->frames.queue.q, framequeue_len);
	/* * */
	encoder->frames.navailable	= io->frames.navailable;
	/* * */
	encoder->frames.post_encoder	= io->frames.post_encoder;
	encoder->frames.ni32_perframe	= io->frames.ni32_perframe;
	encoder->frames.encbuf		= io->frames.encbuf;
	encoder->frames.user		= io->frames.user;
	encoder->frames.enc_retval	= io->frames.enc_retval;

	/* encoder other */
	encoder->fstat			= fstat;

	return;
}

/**@fn encmt_state_free
 * @brief frees any allocated pointers and destroys any objects in the
 *   multi-threaded encoder state structs
 *
 * @param io             - state struct for the io thread
 * @param encoder        - state struct for the encoder threads
 * @param framequeue_len - length of the framequeue
**/
BUILD void
encmt_state_free(
	struct MTArg_EncIO *const RESTRICT io,
	struct MTArg_Encoder *const RESTRICT encoder,
	const unsigned int framequeue_len
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
		io->frames.encbuf[].pcmbuf,
		io->frames.encbuf[].ttabuf
@*/
{
	unsigned int i;

	/* io */
	semaphore_destroy(io->frames.navailable);
	for ( i = 0; i < framequeue_len; ++i ){
		semaphore_destroy(&io->frames.post_encoder[i]);
	}
	for ( i = 0; i < framequeue_len; ++i ){
		codecbuf_free(&io->frames.encbuf[i], CBM_MULTI_THREADED);
	}
	/* * */
	free(io->frames.navailable);

	/* encoder */
	spinlock_destroy(&encoder->frames.queue.lock);

	return;
}

/* ------------------------------------------------------------------------ */

/**@fn encmt_state_init_allocs
 * @brief makes one large allocation and slices it up for the struct pointers
 *
 * @param io             - state struct for the io thread
 * @param framequeue_len - length of the framequeue
**/
static void
encmt_state_init_allocs(
	/*@out@*/ struct MTArg_EncIO *const RESTRICT io,
	const size_t framequeue_len
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		io->frames.navailable,
		io->frames.post_encoder,
		io->frames.ni32_perframe,
		io->frames.encbuf,
		io->frames.user
@*/
/*@allocates	io->frames.navailable@*/
{
	size_t size_total = 0;
	size_t offset[5u];
	uintptr_t base;

	/* navailable */
	size_total += sizeof *io->frames.navailable;
	/* post_encoder */
	size_total += ALIGN_FW_DIFF(
		size_total, ALIGNOF(*io->frames.post_encoder)
	);
	offset[0u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.post_encoder);
	/* ni32_perframe */
	size_total += ALIGN_FW_DIFF(
		size_total, ALIGNOF(*io->frames.ni32_perframe)
	);
	offset[1u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.ni32_perframe);
	/* encbuf */
	size_total += ALIGN_FW_DIFF(size_total, ALIGNOF(*io->frames.encbuf));
	offset[2u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.encbuf);
	/* user */
	size_total += ALIGN_FW_DIFF(size_total, ALIGNOF(*io->frames.user));
	offset[3u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.user);
	/* enc_retval */
	size_total += ALIGN_FW_DIFF(
		size_total, ALIGNOF(*io->frames.enc_retval)
	);
	offset[4u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.enc_retval);

	base = (uintptr_t) calloc_check(SIZE_C(1), size_total);
	io->frames.navailable 		= (void *)  base;
	io->frames.post_encoder		= (void *) (base + offset[0u]);
	io->frames.ni32_perframe	= (void *) (base + offset[1u]);
	io->frames.encbuf		= (void *) (base + offset[2u]);
	io->frames.user			= (void *) (base + offset[3u]);
	io->frames.enc_retval		= (void *) (base + offset[4u]);

	return;
}

/* ======================================================================== */

/**@fn decmt_state_init
 * @brief initializes the multi-threaded decoder state structs
 *
 * @param io             - state struct for the io thread
 * @param decoder        - state struct for the decoder threads
 * @param framequeue_len - length of the framequeue
 * @param i32buf_len     - length of the encbuf->i32buf
 * @param outfile        - destination file
 * @param outfile_name   - name of the destination file (warnings/errors)
 * @param infile         - source file
 * @param infile_name    - name of the source file (warnings/errors)
 * @param seektable      - TTA seektable struct
 * @param dstat_out      - decode stats return struct
 * @param fstat          - compacted file stats struct
**/
BUILD void
decmt_state_init(
	/*@out@*/ struct MTArg_DecIO *const RESTRICT io,
	/*@out@*/ struct MTArg_Decoder *const RESTRICT decoder,
	const unsigned int framequeue_len, const size_t i32buf_len,
	FILE *const RESTRICT outfile, const char *const outfile_name,
	FILE *const RESTRICT infile, const char *const infile_name,
	const struct SeekTable *const RESTRICT seektable,
	const struct DecStats *const RESTRICT dstat_out,
	const struct FileStats_DecMT *const RESTRICT fstat
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
		io->frames.decbuf[].pcmbuf,
		io->frames.decbuf[].ttabuf
@*/
{
	unsigned int i;

	/* base allocations */
	decmt_state_init_allocs(io, (size_t) framequeue_len);

	/* io->frames */
	io->frames.nmemb	= framequeue_len;
	/* * */
	semaphore_init(io->frames.navailable, 0);
	/* * */
	for ( i = 0; i < framequeue_len; ++i ){
		semaphore_init(&io->frames.post_decoder[i], 0);
	}
	for ( i = 0; i < framequeue_len; ++i ){
		decbuf_init(
			&io->frames.decbuf[i], i32buf_len,
			TTABUF_LEN_DEFAULT, fstat->nchan, fstat->samplebytes,
			CBM_MULTI_THREADED
		);
	}

	/* io->outfile */
	io->outfile.handle	= outfile;
	io->outfile.name	= outfile_name;

	/* io->infile */
	io->infile.handle	= infile;
	io->infile.name		= infile_name;

	/* io other */
	io->fstat		= fstat;
	io->seektable		= (struct SeekTable *) seektable;
	io->dstat_out		= (struct DecStats *) dstat_out;

	/* decoder->frames */
	spinlock_init(&decoder->frames.queue.lock);
	pqueue_init(&decoder->frames.queue.q, framequeue_len);
	/* * */
	decoder->frames.navailable          = io->frames.navailable;
	/* * */
	decoder->frames.post_decoder        = io->frames.post_decoder;
	decoder->frames.ni32_perframe       = io->frames.ni32_perframe;
	decoder->frames.nbytes_tta_perframe = io->frames.nbytes_tta_perframe;
	decoder->frames.decbuf              = io->frames.decbuf;
	decoder->frames.user                = io->frames.user;
	decoder->frames.dec_retval          = io->frames.dec_retval;
	decoder->frames.nsamples_flat_2pad  = io->frames.nsamples_flat_2pad;

	/* decoder other */
	decoder->fstat                      = fstat;

	return;
}

/**@fn decmt_state_free
 * @brief frees any allocated pointers and destroys any objects in the
 *   multi-threaded decoder state structs
 *
 * @param io             - state struct for the io thread
 * @param decoder        - state struct for the decoder threads
 * @param framequeue_len - length of the framequeue
**/
BUILD void
decmt_state_free(
	struct MTArg_DecIO *const RESTRICT io,
	struct MTArg_Decoder *const RESTRICT decoder,
	const unsigned int framequeue_len
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
		io->frames.decbuf[].pcmbuf,
		io->frames.decbuf[].ttabuf
@*/
{
	unsigned int i;

	/* io */
	semaphore_destroy(io->frames.navailable);
	for ( i = 0; i < framequeue_len; ++i ){
		semaphore_destroy(&io->frames.post_decoder[i]);
	}
	for ( i = 0; i < framequeue_len; ++i ){
		codecbuf_free(&io->frames.decbuf[i], CBM_MULTI_THREADED);
	}
	/* * */
	free(io->frames.navailable);

	/* decoder */
	spinlock_destroy(&decoder->frames.queue.lock);

	return;
}

/* ------------------------------------------------------------------------ */

/**@fn decmt_state_init_allocs
 * @brief makes one large allocation and slices it up for the struct pointers
 *
 * @param io             - state struct for the io thread
 * @param framequeue_len - length of the framequeue
**/
static void
decmt_state_init_allocs(
	/*@out@*/ struct MTArg_DecIO *const RESTRICT io,
	const size_t framequeue_len
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		io->frames.navailable,
		io->frames.post_decoder,
		io->frames.ni32_perframe,
		io->frames.nbytes_tta_perframe,
		io->frames.decbuf,
		io->frames.user,
		io->frames.nsamples_flat_2pad,
		io->frames.dec_retval,
		io->frames.crc_read
@*/
/*@allocates	io->frames.navailable@*/
{
	size_t size_total = 0;
	size_t offset[8u];
	uintptr_t base;

	/* navailable */
	size_total += sizeof *io->frames.navailable;
	/* post_decoder */
	size_total += ALIGN_FW_DIFF(
		size_total, ALIGNOF(*io->frames.post_decoder)
	);
	offset[0u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.post_decoder);
	/* ni32_perframe */
	size_total += ALIGN_FW_DIFF(
		size_total, ALIGNOF(*io->frames.ni32_perframe)
	);
	offset[1u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.ni32_perframe);
	/* nbytes_tta_perframe */
	size_total += ALIGN_FW_DIFF(
		size_total, ALIGNOF(*io->frames.nbytes_tta_perframe)
	);
	offset[2u]  = size_total;
	size_total += (
		framequeue_len * (sizeof *io->frames.nbytes_tta_perframe)
	);
	/* decbuf */
	size_total += ALIGN_FW_DIFF(size_total, ALIGNOF(*io->frames.decbuf));
	offset[3u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.decbuf);
	/* user */
	size_total += ALIGN_FW_DIFF(size_total, ALIGNOF(*io->frames.user));
	offset[4u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.user);
	/* nsamples_flat_2pad */
	size_total += ALIGN_FW_DIFF(
		size_total, ALIGNOF(*io->frames.nsamples_flat_2pad)
	);
	offset[5u]  = size_total;
	size_total += (
		framequeue_len * (sizeof *io->frames.nsamples_flat_2pad)
	);
	/* dec_retval */
	size_total += ALIGN_FW_DIFF(
		size_total, ALIGNOF(*io->frames.dec_retval)
	);
	offset[6u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.dec_retval);
	/* crc_read */
	size_total += ALIGN_FW_DIFF(
		size_total, ALIGNOF(*io->frames.crc_read)
	);
	offset[7u]  = size_total;
	size_total += framequeue_len * (sizeof *io->frames.crc_read);

	base = (uintptr_t) calloc_check(SIZE_C(1), size_total);
	io->frames.navailable 		= (void *)  base;
	io->frames.post_decoder		= (void *) (base + offset[0u]);
	io->frames.ni32_perframe	= (void *) (base + offset[1u]);
	io->frames.nbytes_tta_perframe	= (void *) (base + offset[2u]);
	io->frames.decbuf		= (void *) (base + offset[3u]);
	io->frames.user			= (void *) (base + offset[4u]);
	io->frames.nsamples_flat_2pad	= (void *) (base + offset[5u]);
	io->frames.dec_retval		= (void *) (base + offset[6u]);
	io->frames.crc_read		= (void *) (base + offset[7u]);

	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
