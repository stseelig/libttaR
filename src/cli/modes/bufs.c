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

#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include "../../bits.h"
#include "../../libttaR.h"

#include "../alloc.h"
#include "../debug.h"

#include "bufs.h"

//////////////////////////////////////////////////////////////////////////////

/**@fn encbuf_init
 * @brief initializes an encbuf
 *
 * @param eb[out] the encode buffers struct
 * @param ni32_len length of the i32buf
 * @param ttabuf_len size of the ttabuf
 * @param nchan number of audio channels
 * @param samplebytes number of bytes per PCM sample
**/
void
encbuf_init(
	/*@out@*/ struct EncBuf *const restrict eb, const size_t ni32_len,
	const size_t ttabuf_len, const uint nchan,
	const enum TTASampleBytes samplebytes, enum CodecBufMode mode
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*eb
@*/
/*@allocates	eb->i32buf,
		eb->pcmbuf,
		eb->ttabuf
@*/
{
	const size_t safety_margin = libttaR_ttabuf_safety_margin(
		samplebytes, nchan
	);
	assert(safety_margin != 0);

	eb->i32buf_len = ni32_len;
	assert(eb->i32buf_len != 0);
	eb->ttabuf_len = (ttabuf_len * nchan) + safety_margin;
	assert(eb->ttabuf_len != 0);

	switch ( mode ){
	case CBM_SINGLE_THREADED:
		eb->i32buf = calloc_check(eb->i32buf_len, sizeof *eb->i32buf);
		break;
	case CBM_MULTI_THREADED:
		eb->i32buf = NULL;
		break;
	}
	eb->pcmbuf = calloc_check(eb->i32buf_len, (size_t) samplebytes);
	eb->ttabuf = malloc_check(eb->ttabuf_len);
	return;
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
	eb->ttabuf = realloc_check(eb->ttabuf, eb->ttabuf_len);
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
**/
void
decbuf_init(
	/*@out@*/ struct DecBuf *const restrict db, const size_t ni32_len,
	const size_t ttabuf_len, const uint nchan,
	const enum TTASampleBytes samplebytes, enum CodecBufMode mode
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*db
@*/
/*@allocates	db->i32buf,
		db->pcmbuf,
		db->ttabuf
@*/
{
	const size_t safety_margin = libttaR_ttabuf_safety_margin(
		samplebytes, nchan
	);
	assert(safety_margin != 0);

	db->i32buf_len = ni32_len;
	assert(db->i32buf_len != 0);
	db->ttabuf_len = (ttabuf_len * nchan) + safety_margin;
	assert(db->ttabuf_len != 0);

	switch ( mode ){
	case CBM_SINGLE_THREADED:
		db->i32buf = calloc_check(ni32_len, sizeof *db->i32buf);
		break;
	case CBM_MULTI_THREADED:
		db->i32buf = NULL;
		break;
	}
	db->pcmbuf = calloc_check(ni32_len, (size_t) samplebytes);
	db->ttabuf = malloc_check(db->ttabuf_len);
	return;
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
		db->ttabuf     = realloc_check(db->ttabuf, newsize);
	}
	return;
}

//==========================================================================//

/**@fn codecbuf_free
 * @brief free any allocated pointers in a codecbuf
 *
 * @param cb[in] the codec buffers struct
**/
void
codecbuf_free(
	const struct CodecBuf *const restrict cb, enum CodecBufMode mode
)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	cb->i32buf,
		cb->pcmbuf,
		cb->ttabuf
@*/
{
	if ( mode == CBM_SINGLE_THREADED ){
		free(cb->i32buf);
	}
	free(cb->pcmbuf);
	free(cb->ttabuf);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
