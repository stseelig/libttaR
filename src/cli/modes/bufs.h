#ifndef TTA_MODES_BUFS_H
#define TTA_MODES_BUFS_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/bufs.h                                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>

#include "../../bits.h"

#include "../formats.h"	// enum TTASampleBytes

//////////////////////////////////////////////////////////////////////////////

#define TTABUF_LEN_DEFAULT		((size_t) BUFSIZ)

//////////////////////////////////////////////////////////////////////////////

struct EncBuf {
	size_t	i32buf_len;
	size_t	ttabuf_len;
	/*@only@*/
	i32	*i32buf;
	/*@only@*/
	u8	*pcmbuf;
	/*@only@*/
	u8	*ttabuf;
};

struct DecBuf {
	size_t	i32buf_len;
	size_t	ttabuf_len;
	/*@only@*/
	i32	*i32buf;
	/*@only@*/
	u8	*pcmbuf;
	/*@only@*/
	u8	*ttabuf;
};

//////////////////////////////////////////////////////////////////////////////

#undef eb
extern void encbuf_init(
	/*@out@*/ struct EncBuf *const restrict eb, size_t, size_t, uint,
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
		eb->pcmbuf,
		eb->ttabuf
@*/
;

#undef eb
extern HOT void encbuf_adjust(struct EncBuf *const restrict eb, size_t, uint)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		eb->ttabuf_len,
		eb->ttabuf
@*/
;

//--------------------------------------------------------------------------//

#undef db
extern void decbuf_init(
	/*@out@*/ struct DecBuf *const restrict db, size_t, size_t, uint,
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
		db->i32buf,
		db->ttabuf
@*/
;

#undef db
extern HOT void decbuf_check_adjust(
	struct DecBuf *const restrict db, size_t, uint,
	enum TTASampleBytes
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		db->ttabuf_len,
		db->ttabuf
@*/
;

//--------------------------------------------------------------------------//

#undef cb
extern void codecbuf_free(const struct EncBuf *const restrict cb)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	cb->pcmbuf,
		cb->i32buf,
		cb->ttabuf
@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
