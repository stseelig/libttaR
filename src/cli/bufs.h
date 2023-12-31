#ifndef TTA_BUFS_H
#define TTA_BUFS_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec.h                                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>

#include "../bits.h"

#include "formats.h"	// enum TTASampleBytes

//////////////////////////////////////////////////////////////////////////////

struct EncBuf {
	size_t	i32buf_len;
	size_t	ttabuf_len;
	/*@only@*/ /*@relnull@*/
	i32	*i32buf;
	/*@dependent@*/ /*@relnull@*/
	u8	*pcmbuf;
	/*@only@*/ /*@relnull@*/
	u8	*ttabuf;
};

struct DecBuf {
	size_t	i32buf_len;
	size_t	ttabuf_len;
	/*@only@*/ /*@relnull@*/
	u8	*pcmbuf;
	/*@dependent@*/ /*@relnull@*/
	i32	*i32buf;
	/*@only@*/ /*@relnull@*/
	u8	*ttabuf;
};

//////////////////////////////////////////////////////////////////////////////

#undef eb
extern size_t encbuf_init(
	/*@out@*/ struct EncBuf *const restrict eb, size_t, uint,
	enum TTASampleBytes
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
;

#undef eb
extern void encbuf_free(struct EncBuf *const restrict eb)
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
;

#undef db
extern size_t decbuf_init(
	/*@out@*/ struct DecBuf *const restrict db, size_t, uint,
	enum TTASampleBytes
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
;

#undef db
extern void decbuf_free(struct DecBuf *const restrict db)
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
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
