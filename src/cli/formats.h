#ifndef TTA_FORMATS_H
#define TTA_FORMATS_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats.h                                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t
#include <stdio.h>	// off_t

#include "../bits.h"

//////////////////////////////////////////////////////////////////////////////

#include "formats/guid.h"
#include "formats/tta.h"

//////////////////////////////////////////////////////////////////////////////

enum EncFormat {
	xENCFMT_TTA1
	//xENCFMT_TTA2
	//xENCFMT_MKA_TTA1
	//xENCFMT_MKA_TTA2
};
#define NUM_ENCFMT		((uint) 1u)
#define xENCFMT_NAME_ARRAY	{ "tta1"}
#define xENCFMT_EXT_ARRAY	{".tta"}

enum DecFormat {
	DECFMT_RAWPCM,
	DECFMT_W64,
	DECFMT_WAV
	//DECFMT_MKA_PCM
};
#define NUM_DECFMT		((uint) 3u)
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

//////////////////////////////////////////////////////////////////////////////

struct SeekTable {
	off_t	off;
	size_t	nmemb;
	size_t	limit;
	/*@only@*/
	u32	*table;	// little-endian
};
#define SEEKTABLE_INIT_DEFAULT	((size_t) (512u))	/* ~ 8m54s */

// MAYBE have a Dec/Enc FileStats and a CommonFileStats
struct FileStats {
	off_t			decpcm_off;
	size_t			decpcm_size;
	//
	off_t			enctta_off;	// end of tta header
	size_t			enctta_size;
	//
	size_t			framelen;	// nsamples_peridealframe
	size_t			buflen;		// framelen * nchan
	size_t			nsamples_enc;	// for decode
	enum EncFormat		encfmt  : 8u;
	enum DecFormat		decfmt  : 8u;
	enum IntType		inttype : 8u;
	enum Endian		endian  : 8u;
	enum LibTTAr_SampleBytes	samplebytes;
	u16			samplebits;
	u16			nchan;
	u32			samplerate;
	u32			chanmask_wav;
	//u32			chanmask_tta;
	u16			wavformat;
	struct Guid128		wavsubformat;	// Extensible only
};

struct EncStats {
	size_t	nframes;
	size_t	nsamples_flat;
	size_t	nsamples_perchan;	// for tta1 header
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

//////////////////////////////////////////////////////////////////////////////

// guid.c

#undef buf
extern COLD char *guid128_format(
	/*@returned@*/ /*@out@*/ char *restrict buf, size_t,
	const struct Guid128 *restrict
)
/*@modifies	*buf@*/
;

//--------------------------------------------------------------------------//

// metatags_skip.c

#undef file
extern enum FileCheck metatags_skip(FILE *restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

//--------------------------------------------------------------------------//

// tta1_check

#undef fstat
#undef file
extern enum FileCheck filecheck_tta1(
	/*@out@*/ struct FileStats *restrict fstat, FILE *restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
;

//--------------------------------------------------------------------------//

// tta_seek.c

extern CONST size_t seektable_nframes(size_t, size_t, uint) /*@*/;

#undef st
extern void seektable_init(/*@out@*/ struct SeekTable *restrict st, size_t)
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
extern HOT void seektable_add(
	struct SeekTable *restrict st, size_t, const char *restrict
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
extern void seektable_free(const struct SeekTable *restrict st)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	st->table@*/
;

//--------------------------------------------------------------------------//

#undef st
#undef file
extern enum FileCheck filecheck_tta_seektable(
	/*@out@*/ struct SeekTable *restrict st, size_t, FILE *restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*st,
		*fstat,
		file
@*/
;

//--------------------------------------------------------------------------//

// tta_write.c

#undef outfile
extern void prewrite_tta1_header_seektable(
	FILE *restrict outfile, const struct SeekTable *restrict,
	const char *restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
extern off_t write_tta1_header(
	FILE *restrict outfile, size_t, const struct FileStats *restrict,
	const char *
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
extern void write_tta_seektable(
	FILE *restrict outfile, const struct SeekTable *restrict,
	const char *restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

//--------------------------------------------------------------------------//

// w64_check.c

#undef fstat
#undef file
extern enum FileCheck filecheck_w64(
	/*@out@*/ struct FileStats *restrict fstat, FILE *restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
;

//--------------------------------------------------------------------------//

// w64_write.c

#undef outfile
extern void prewrite_w64_header(FILE *restrict outfile, const char *)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
extern void write_w64_header(
	FILE *restrict outfile, size_t, const struct FileStats *restrict,
	const char *restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

//--------------------------------------------------------------------------//

// wav_check.c

#undef fstat
#undef file
extern enum FileCheck filecheck_wav(
	/*@out@*/ struct FileStats *restrict fstat, FILE *restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
;

#undef fstat
#undef file
extern enum FileCheck filecheck_wav_read_subchunk_fmt(
	/*@out@*/ struct FileStats *restrict fstat, FILE *restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
;

//--------------------------------------------------------------------------//

// wav_write.c

#undef outfile
extern void prewrite_wav_header(FILE *restrict outfile, const char *restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
extern void write_wav_header(
	FILE *restrict outfile, size_t, const struct FileStats *restrict,
	const char *restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef body
extern void fill_waveformatex_body(
	/*@out@*/ struct RiffSubChunk_WaveFormatEX_Body *restrict body,
	u16, const struct FileStats *restrict
)
/*@modifies	*body@*/
;

#undef wfx
extern void fill_waveformatextensible(
	/*@out@*/
	struct RiffSubChunk_WaveFormatExtensible_Tail *restrict wfx,
	const struct FileStats *restrict
)
/*@modifies	*wfx@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
