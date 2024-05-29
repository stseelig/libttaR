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
#include <string.h>

#include "../../bits.h"
#include "../../libttaR.h"

#include "../debug.h"

#include "bufs.h"

//////////////////////////////////////////////////////////////////////////////

#define BUF_ALIGN	((size_t) 16u)

//////////////////////////////////////////////////////////////////////////////

// returns (ni32_samplebuf_len * nchan) (for sanity check)
size_t
encbuf_init(
	/*@out@*/ struct EncBuf *const restrict eb, size_t ni32_samplebuf_len,
	size_t ttabuf_len, uint nchan, enum TTASampleBytes samplebytes
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
	const size_t r = (size_t) (ni32_samplebuf_len * nchan);
	const size_t pcmbuf_size = (size_t) (r * samplebytes);
	union {	uintptr_t p; } t;

	eb->i32buf_len = r + BUF_ALIGN;
	assert(eb->i32buf_len != 0);
	eb->ttabuf_len = ttabuf_len + libttaR_ttabuf_safety_margin(nchan);
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

// in encode loop
HOT void
encbuf_adjust(struct EncBuf *const restrict eb, size_t samplebuf_len)
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
	eb->ttabuf_len += samplebuf_len;
	assert(eb->ttabuf_len != 0);

	eb->ttabuf = realloc(eb->ttabuf, eb->ttabuf_len);
	if UNLIKELY ( eb->ttabuf == NULL ){
		error_sys(errno, "realloc", NULL);
	}
	assert(eb->ttabuf != NULL);

	return;
}

void
encbuf_free(struct EncBuf *const restrict eb)
/*@globals	internalState@*/
/*@modifies	internalState,
		*eb
@*/
/*@releases	eb->i32buf,
		eb->ttabuf
@*/
{
	free(eb->i32buf);
	free(eb->ttabuf);
	return;
}

//==========================================================================//

// returns (ni32_samplebuf_len * nchan) (for sanity check)
size_t
decbuf_init(
	/*@out@*/ struct DecBuf *const restrict db, size_t ni32_samplebuf_len,
	size_t ttabuf_len, uint nchan
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
	const size_t r = (size_t) (ni32_samplebuf_len * nchan);

	db->i32buf_len = r;
	assert(db->i32buf_len != 0);
	db->ttabuf_len = ttabuf_len + libttaR_ttabuf_safety_margin(nchan);
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

// in decode loop
HOT void
decbuf_check_adjust(
	struct DecBuf *const restrict db, size_t newsize, uint nchan
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
	// the safety-margin needs to be re-applied here
	newsize += libttaR_ttabuf_safety_margin(nchan);
	assert(newsize != 0);

	if ( newsize > db->ttabuf_len ){
		db->ttabuf_len = newsize;
		db->ttabuf     = realloc(db->ttabuf, newsize);
		if UNLIKELY ( db->ttabuf == NULL ){
			error_sys(errno, "realloc", NULL);
		}
		assert(db->ttabuf != NULL);
	}

	return;
}

void
decbuf_free(struct DecBuf *const restrict db)
/*@globals	internalState@*/
/*@modifies	internalState,
		*db
@*/
/*@releases	db->pcmbuf,
		db->ttabuf
@*/
{
	free(db->pcmbuf);
	free(db->ttabuf);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
