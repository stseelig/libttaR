//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// bufs.c                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// i32buf and pcmbuf overlap to save space                                  //
// all pointers are aligned to 16                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../bits.h"
#include "../libttaR.h"

#include "bufs.h"	// struct EncBuf, struct DecBuf
#include "debug.h"

//////////////////////////////////////////////////////////////////////////////

#define I32BUF_SAFETY_MARGIN	((size_t) 16)

//////////////////////////////////////////////////////////////////////////////

// returns buflen (for sanity check)
size_t
encbuf_init(
	/*@out@*/ struct EncBuf *const restrict eb, size_t buflen,
	uint nchan, enum TTASampleBytes samplebytes
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
	size_t r;
	size_t pcmbuf_size;
	union {
		uintptr_t p;
	} t;

	r = (size_t) (buflen * nchan);

	eb->i32buf_len = r + I32BUF_SAFETY_MARGIN;
	eb->ttabuf_len = libttaR_ttabuf_size(buflen, nchan, samplebytes);
	assert(eb->ttabuf_len != 0);

	eb->i32buf = calloc(eb->i32buf_len, sizeof(i32));
	if ( eb->i32buf == NULL ){
		error_sys(errno, "calloc", strerror(errno), NULL);
	}
	assert(eb->i32buf != NULL);

	pcmbuf_size = (size_t) (r * samplebytes);
	t.p	    = (uintptr_t) &eb->i32buf[eb->i32buf_len];
	t.p        -= pcmbuf_size + 1u;
	eb->pcmbuf  =  (u8 *) (t.p - (t.p % 16u));

	eb->ttabuf = malloc(eb->ttabuf_len);
	if ( eb->ttabuf == NULL ){
		error_sys(errno, "malloc", strerror(errno), NULL);
	}
	assert(eb->ttabuf != NULL);

	return r;
}

void
encbuf_free(struct EncBuf *const restrict eb)
/*@globals		internalState@*/
/*@modifies		internalState,
			*eb
@*/
/*@releases		eb->i32buf,
			eb->ttabuf
@*/
/*@ensures isnull	eb->ttabuf,
			eb->i32buf,
			eb->pcmbuf
@*/
{
	if ( eb->i32buf != NULL ){
		free(eb->i32buf);
	}
	if ( eb->ttabuf != NULL ){
		free(eb->ttabuf);
	}
	(void) memset(eb, 0x00, sizeof *eb);
	return;
}

//==========================================================================//

// TODO fix for new tta2dec
// returns buflen (for sanity check)
size_t
decbuf_init(
	/*@out@*/ struct DecBuf *const restrict db, size_t buflen, uint nchan,
	enum TTASampleBytes samplebytes
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
	const size_t r = buflen * nchan;

	db->i32buf_len = r;
	db->ttabuf_len = libttaR_ttabuf_size(buflen, nchan, samplebytes);
	assert(db->ttabuf_len != 0);

	db->pcmbuf = calloc(r + I32BUF_SAFETY_MARGIN, sizeof(i32));
	if ( db->pcmbuf == NULL ){
		error_sys(errno, "calloc", strerror(errno), NULL);
	}
	assert(db->pcmbuf != NULL);

	db->i32buf = (i32 *) &db->pcmbuf[I32BUF_SAFETY_MARGIN * sizeof(i32)];

	db->ttabuf = malloc(db->ttabuf_len);
	if ( db->ttabuf == NULL ){
		error_sys(errno, "malloc", strerror(errno), NULL);
	}
	assert(db->ttabuf != NULL);

	return r;
}

void
decbuf_free(struct DecBuf *const restrict db)
/*@globals		internalState@*/
/*@modifies		internalState,
			*db
@*/
/*@releases		db->pcmbuf,
			db->ttabuf
@*/
/*@ensures isnull	db->pcmbuf,
			db->i32buf,
			db->ttabuf
@*/
{
	if ( db->pcmbuf != NULL ){
		free(db->pcmbuf);
	}
	if ( db->pcmbuf != NULL ){
		free(db->ttabuf);
	}
	(void) memset(db, 0x00, sizeof *db);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
