#ifndef H_TTA_FORMATS_H
#define H_TTA_FORMATS_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats.h                                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "../libttaR.h"

#include "./common.h"

#include "./formats/guid.h"
#include "./formats/tta.h"
#include "./formats/wav.h"

/* //////////////////////////////////////////////////////////////////////// */

enum EncFormat {
	xENCFMT_TTA1
/*
	xENCFMT_TTA2
	xENCFMT_MKA_TTA1
	xENCFMT_MKA_TTA2
*/
};
#define xENCFMT_NMEMB		1u
#define xENCFMT_NAME_ARRAY	{ "tta1"}
#define xENCFMT_EXT_ARRAY	{".tta" }

enum DecFormat {
	DECFMT_RAWPCM,
	DECFMT_W64,
	DECFMT_WAV
/*
	DECFMT_MKA_PCM
*/
};
#define DECFMT_NMEMB		3u
#define DECFMT_NAME_ARRAY	{ "raw",  "w64",  "wav"}
#define DECFMT_EXT_ARRAY	{".raw", ".w64", ".wav"}

enum IntType {
	INT_SIGNED,
	INT_UNSIGNED
};

enum Endian {
	xENDIAN_BIG,
	xENDIAN_LITTLE
};

enum FileCheck {
	FILECHECK_OK = 0,
	FILECHECK_READ_ERROR,
	FILECHECK_SEEK_ERROR,
	FILECHECK_MISMATCH,
	FILECHECK_MALFORMED,
	FILECHECK_CORRUPTED,
	FILECHECK_UNSUPPORTED_DATATYPE,
	FILECHECK_UNSUPPORTED_RESOLUTION
};

/* //////////////////////////////////////////////////////////////////////// */

struct SeekTable {
	off_t		off;
	size_t		nmemb;
	size_t		limit;
	/*@only@*/
	uint32_t	*table;	/* little-endian */
};
#define SEEKTABLE_INIT_DEFAULT	SIZE_C(512)	/* ~ 8m54s */

/* ======================================================================== */

/* MAYBE: have a Dec/Enc FileStats and a CommonFileStats */
struct FileStats {
	off_t			decpcm_off;
	size_t			decpcm_size;

	off_t			enctta_off;	/* end of TTA header      */
	size_t			enctta_size;

	size_t			framelen;	/* nsamples_peridealframe */
	size_t			buflen;		/* (framelen * nchan)     */
	size_t			nsamples_enc;	/* for decode             */
	enum EncFormat		encfmt  : 8u;
	enum DecFormat		decfmt  : 8u;
	enum IntType		inttype : 8u;
	enum Endian		endian  : 8u;
	enum LibTTAr_SampleBytes	samplebytes;
	uint16_t		samplebits;
	uint16_t		nchan;
	uint32_t		samplerate;
/*
	uint32_t		chanmask_tta;
*/
	uint32_t		chanmask_wav;
	uint16_t		wavformat;
	struct Guid128		wavsubformat;	/* Extensible only        */
};

struct EncStats {
	size_t	nframes;
	size_t	nsamples_flat;
	size_t	nsamples_perchan;	/* for TTA1 header        */
	size_t	nbytes_encoded;
	double	encodetime;
};

struct DecStats {
	size_t	nframes;
	size_t	nsamples_flat;
	size_t	nsamples_perchan;
	size_t	nbytes_decoded;
	double	decodetime;
};

/* //////////////////////////////////////////////////////////////////////// */

/* guid.c */

#undef buf
COLD
/*@temp@*/
BUILD_EXTERN char *guid128_format(
	/*@returned@*/ /*@out@*/ char *RESTRICT buf, size_t,
	const struct Guid128 *RESTRICT
)
/*@modifies	*buf@*/
;

/* ------------------------------------------------------------------------ */

/* metatags_skip.c */

#undef file
BUILD_EXTERN enum FileCheck metatags_skip(FILE *RESTRICT file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

/* ------------------------------------------------------------------------ */

/* tta1_check.c */

#undef fstat
#undef file
BUILD_EXTERN enum FileCheck filecheck_tta1(
	/*@out@*/ struct FileStats *RESTRICT fstat, FILE *RESTRICT file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
;

/* ------------------------------------------------------------------------ */

/* tta_seek.c */

CONST
BUILD_EXTERN size_t seektable_nframes(size_t, size_t, unsigned int) /*@*/;

#undef st
BUILD_EXTERN void seektable_init(
	/*@out@*/ struct SeekTable *RESTRICT st, size_t
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*st
@*/
/*@allocates	st->table@*/
;

#undef st
BUILD_EXTERN HOT void seektable_add(
	struct SeekTable *RESTRICT st, size_t, const char *RESTRICT
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*st
@*/
;

#undef st
BUILD_EXTERN void seektable_free(const struct SeekTable *RESTRICT st)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	st->table@*/
;

/* ------------------------------------------------------------------------ */

/* tta_seek_check.c */

#undef st
#undef file
BUILD_EXTERN enum FileCheck filecheck_tta_seektable(
	/*@out@*/ struct SeekTable *RESTRICT st, size_t, FILE *RESTRICT file
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*st,
		*fstat,
		file
@*/
;

/* ------------------------------------------------------------------------ */

/* tta_write.c */

#undef outfile
BUILD_EXTERN void prewrite_tta1_header_seektable(
	FILE *RESTRICT outfile, const struct SeekTable *RESTRICT,
	const char *RESTRICT
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
BUILD_EXTERN off_t write_tta1_header(
	FILE *RESTRICT outfile, size_t, const struct FileStats *RESTRICT,
	const char *
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
BUILD_EXTERN void write_tta_seektable(
	FILE *RESTRICT outfile, const struct SeekTable *RESTRICT,
	const char *RESTRICT
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

/* ------------------------------------------------------------------------ */

/* w64_check.c */

#undef fstat
#undef file
BUILD_EXTERN enum FileCheck filecheck_w64(
	/*@out@*/ struct FileStats *RESTRICT fstat, FILE *RESTRICT file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
;

/* ------------------------------------------------------------------------ */

/* w64_write.c */

#undef outfile
BUILD_EXTERN void prewrite_w64_header(FILE *RESTRICT outfile, const char *)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
BUILD_EXTERN void write_w64_header(
	FILE *RESTRICT outfile, size_t, const struct FileStats *RESTRICT,
	const char *RESTRICT
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

/* ------------------------------------------------------------------------ */

/* wav_check.c */

#undef fstat
#undef file
BUILD_EXTERN enum FileCheck filecheck_wav(
	/*@out@*/ struct FileStats *RESTRICT fstat, FILE *RESTRICT file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
;

#undef fstat
#undef file
BUILD_EXTERN enum FileCheck filecheck_wav_read_subchunk_fmt(
	/*@out@*/ struct FileStats *RESTRICT fstat, FILE *RESTRICT file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
;

/* ------------------------------------------------------------------------ */

/* wav_write.c */

#undef outfile
BUILD_EXTERN void prewrite_wav_header(
	FILE *RESTRICT outfile, const char *RESTRICT
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
BUILD_EXTERN void write_wav_header(
	FILE *RESTRICT outfile, size_t, const struct FileStats *RESTRICT,
	const char *RESTRICT
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef body
BUILD_EXTERN void fill_waveformatex_body(
	/*@out@*/ struct RiffSubChunk_WaveFormatEX_Body *RESTRICT body,
	uint16_t, const struct FileStats *RESTRICT
)
/*@modifies	*body@*/
;

#undef wfx
BUILD_EXTERN void fill_waveformatextensible(
	/*@out@*/ struct RiffSubChunk_WaveFormatExtensible_Tail *RESTRICT wfx,
	const struct FileStats *RESTRICT
)
/*@modifies	*wfx@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_FORMATS_H */
