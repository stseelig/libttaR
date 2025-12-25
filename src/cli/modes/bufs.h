#ifndef H_TTA_MODES_BUFS_H
#define H_TTA_MODES_BUFS_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/bufs.h                                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "../../libttaR.h"

#include "../alloc.h"
#include "../common.h"
#include "../formats.h"

#include "./align.h"

/* //////////////////////////////////////////////////////////////////////// */

#define TTABUF_LEN_DEFAULT		((size_t) BUFSIZ)

enum CodecBufMode {
	CBM_SINGLE_THREADED,
	CBM_MULTI_THREADED
};

/* //////////////////////////////////////////////////////////////////////// */

struct CodecBuf {
	size_t	i32buf_len;
	size_t	ttabuf_len;
	/*@only@*/ /*@null@*/	/* @only@ in -S, @dependent@ in -M          */
	int32_t	*i32buf;	/* thread owned in -M; page-fault reduction */
	/*@only@*/
	uint8_t	*pcmbuf;
	/*@only@*/
	uint8_t	*ttabuf;
};

#define EncBuf	CodecBuf
#define DecBuf	CodecBuf

/* //////////////////////////////////////////////////////////////////////// */

#undef eb
BUILD_EXTERN NOINLINE void encbuf_init(
	/*@out@*/ struct EncBuf *const RESTRICT eb, size_t, size_t,
	unsigned int, enum LibTTAr_SampleBytes, enum CodecBufMode
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
HOT
BUILD_EXTERN void encbuf_adjust(
	struct EncBuf *const RESTRICT eb, size_t, unsigned int
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		eb->ttabuf_len,
		eb->ttabuf
@*/
;

/* ------------------------------------------------------------------------ */

#undef db
BUILD_EXTERN NOINLINE void decbuf_init(
	/*@out@*/ struct DecBuf *const RESTRICT db, size_t, size_t,
	unsigned int, enum LibTTAr_SampleBytes, enum CodecBufMode
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
;

#undef db
HOT
BUILD_EXTERN void decbuf_check_adjust(
	struct DecBuf *const RESTRICT db, size_t, unsigned int,
	enum LibTTAr_SampleBytes
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

/* ------------------------------------------------------------------------ */

#undef cb
BUILD_EXTERN NOINLINE void codecbuf_free(
	const struct CodecBuf *const RESTRICT cb, enum CodecBufMode
)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	cb->i32buf,
		cb->pcmbuf,
		cb->ttabuf
@*/
;

/* ------------------------------------------------------------------------ */

/*@only@*/
BUILD_EXTERN struct LibTTAr_CodecState_Priv * priv_alloc(const unsigned int)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;

#undef ptr
BUILD_EXTERN void priv_free(
	/*@only@*/ struct LibTTAr_CodecState_Priv *const RESTRICT ptr
)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	*ptr@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_MODES_BUFS_H */
