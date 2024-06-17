//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/bufs.c                                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      i32buf and pcmbuf overlap to save space                             //
//      all pointers are aligned to 16                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdint.h>	// uintptr_t
#include <string.h>

#include "../../bits.h"
#include "../../libttaR.h"

#include "../debug.h"

#include "bufs.h"

//////////////////////////////////////////////////////////////////////////////

#define BUF_ALIGN	((size_t) 16u)

//////////////////////////////////////////////////////////////////////////////

/**@fn encbuf_init
 * @brief initializes an encbuf
 *
 * @param eb[out] the encode buffers struct
 * @param ni32_len length of the i32buf
 * @param ttabuf_len size of the ttabuf
 * @param nchan number of audio channels
 * @param samplebytes number of bytes per PCM sample
 *
 * @return (ni32_len * nchan) for sanity check
**/
size_t
encbuf_init(
	/*@out@*/ struct EncBuf *const restrict eb, const size_t ni32_len,
	const size_t ttabuf_len, const uint nchan,
	const enum TTASampleBytes samplebytes
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*eb
@*/
/*@allocates	eb->i32buf,
		eb->ttabuf
@*/
{
	const size_t r = (size_t) (ni32_len * nchan);
	const size_t pcmbuf_size = (size_t) (r * samplebytes);
	const size_t safety_margin = libttaR_ttabuf_safety_margin(
		samplebytes, nchan
	);
	union {	uintptr_t p; } t;

	assert(safety_margin != 0);

	eb->i32buf_len = r + BUF_ALIGN;
	assert(eb->i32buf_len != 0);
	eb->ttabuf_len = (ttabuf_len * nchan) + safety_margin;
	assert(eb->ttabuf_len != 0);

	eb->i32buf = calloc(eb->i32buf_len, sizeof *eb->i32buf);
	if UNLIKELY ( eb->i32buf == NULL ){
		error_sys(errno, "calloc", NULL);
	}
	assert(eb->i32buf != NULL);

	t.p	    = (uintptr_t) &eb->i32buf[eb->i32buf_len];
	t.p        -= pcmbuf_size + 1u;
	eb->pcmbuf  = (u8 *) (t.p - (t.p % BUF_ALIGN));

	eb->ttabuf = malloc(eb->ttabuf_len);
	if UNLIKELY ( eb->ttabuf == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(eb->ttabuf != NULL);

	return r;
}

/**@fn encbuf_adjust
 * @brief adjust an encbuf
 *
 * @param eb[in out] the encode buffers struct
 * @param add_len additional size for the ttabuf
 * @param nchan number of audio channels
 *
 * @note in encode loop
**/
HOT void
encbuf_adjust(
	struct EncBuf *const restrict eb, const size_t add_len,
	const uint nchan
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		eb->ttabuf_len,
		eb->ttabuf
@*/
{
	// the safety-margin should have already been added by here
	eb->ttabuf_len += (add_len * nchan);
	assert(eb->ttabuf_len != 0);

	eb->ttabuf = realloc(eb->ttabuf, eb->ttabuf_len);
	if UNLIKELY ( eb->ttabuf == NULL ){
		error_sys(errno, "realloc", NULL);
	}
	assert(eb->ttabuf != NULL);

	return;
}

/**@fn encbuf_free
 * @brief free any allocated pointers in an encbuf
 *
 * @param eb[in] the encode buffers struct
**/
void
encbuf_free(const struct EncBuf *const restrict eb)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	eb->i32buf,
		eb->ttabuf
@*/
{
	free(eb->i32buf);
	free(eb->ttabuf);
	return;
}

//==========================================================================//

/**@fn decbuf_init
 * @brief initializes a decbuf
 *
 * @param db[out] the decode buffers struct
 * @param ni32_len length of the i32buf
 * @param ttabuf_len size of the ttabuf
 * @param nchan number of audio channels
 * @param samplebytes number of bytes per PCM sample
 *
 * @return (ni32_len * nchan) for sanity check
**/
size_t
decbuf_init(
	/*@out@*/ struct DecBuf *const restrict db, const size_t ni32_len,
	const size_t ttabuf_len, const uint nchan,
	const enum TTASampleBytes samplebytes
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*db
@*/
/*@allocates	db->pcmbuf,
		db->ttabuf
@*/
{
	const size_t r = (size_t) (ni32_len * nchan);
	const size_t safety_margin = libttaR_ttabuf_safety_margin(
		samplebytes, nchan
	);

	assert(safety_margin != 0);

	db->i32buf_len = r;
	assert(db->i32buf_len != 0);
	db->ttabuf_len = ttabuf_len + safety_margin;
	assert(db->ttabuf_len != 0);

	db->pcmbuf = calloc(r + BUF_ALIGN, sizeof(i32));
	if UNLIKELY ( db->pcmbuf == NULL ){
		error_sys(errno, "calloc", NULL);
	}
	assert(db->pcmbuf != NULL);

	db->i32buf = (i32 *) &db->pcmbuf[BUF_ALIGN * sizeof(i32)];

	db->ttabuf = malloc(db->ttabuf_len);
	if UNLIKELY ( db->ttabuf == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(db->ttabuf != NULL);

	return r;
}

/**@fn decbuf_check_adjust
 * @brief check and maybe adjust a decbuf
 *
 * @param db[in out] the decode buffers struct
 * @param newsize new ttabuf size
 * @param nchan number of audio channels
 * @param samplebytes number of bytes per PCM sample
 *
 * @note in decode loop
**/
HOT void
decbuf_check_adjust(
	struct DecBuf *const restrict db, size_t newsize, const uint nchan,
	const enum TTASampleBytes samplebytes
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		db->ttabuf_len,
		db->ttabuf
@*/
{
	const size_t safety_margin = libttaR_ttabuf_safety_margin(
		samplebytes, nchan
	);

	assert(safety_margin != 0);

	// the safety-margin needs to be re-applied here
	newsize += safety_margin;
	assert(newsize != 0);

	if ( newsize > db->ttabuf_len ){
		db->ttabuf_len = newsize;
		db->ttabuf     = realloc(db->ttabuf, newsize);
		if UNLIKELY ( db->ttabuf == NULL ){
			error_sys(errno, "realloc", NULL);
		}
	}
	assert(db->ttabuf != NULL);

	return;
}

/**@fn decbuf_free
 * @brief free any allocated pointers in a decbuf
 *
 * @param db[in] the decode buffers struct
**/
void
decbuf_free(const struct DecBuf *const restrict db)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	db->pcmbuf,
		db->ttabuf
@*/
{
	free(db->pcmbuf);
	free(db->ttabuf);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
