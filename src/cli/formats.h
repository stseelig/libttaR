#ifndef TTA_FORMATS_H
#define TTA_FORMATS_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats.h                                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>	// off_t

#include "../bits.h"

//////////////////////////////////////////////////////////////////////////////

#include "formats/guid.h"
#include "formats/tta.h"

//////////////////////////////////////////////////////////////////////////////

enum EncFormat {
	FORMAT_TTA1,
	//FORMAT_TTA2,
	//FORMAT_MKA_TTA1,
	//FORMAT_MKA_TTA2
};

enum DecFormat {
	FORMAT_RAWPCM,
	FORMAT_WAV,		// Microsoft RIFF/WAVE
	FORMAT_W64,		// Sony Wave64
	//FORMAT_MKA_PCM
};

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

#define SEEKTABLE_INIT_DEFAULT	((size_t) (60*60))	/* about one hour */
struct SeekTable {
	off_t	off;
	size_t	nmemb;
	size_t	limit;
	/*@only@*/ /*@relnull@*/
	u32	*table;
};

// MAYBE have a Dec/Enc FileStats and a CommonFileStats
struct FileStats {
	off_t			decpcm_off;
	size_t			decpcm_size;
	//
	off_t			enctta_off;	// end of tta header
	size_t			enctta_size;
	//
	size_t			framelen;
	size_t			buflen;		// framelen * nchan
	enum EncFormat		encfmt:8;
	enum DecFormat		decfmt:8;
	enum IntType		inttype:8;
	u16			samplebits:8;
	enum TTASampleBytes	samplebytes:8;
	enum Endian		endian:8;
	u16			nchan;
	u32			samplerate;
	u32			chanmask_wav;
	//u32			chanmask_tta;
	size_t			nsamples;	// for decode
	u16			wavformat;
	struct Guid128		wavsubformat;	// Extensible only
};

struct EncStats {
	double	encodetime;
	size_t	nframes;
	size_t	nsamples;
	size_t	nsamples_perchan;	// for overflow protection
	size_t	nbytes_encoded;
};

struct DecStats {
	double	decodetime;
	size_t	nframes;
	size_t	nsamples;
	size_t	nsamples_perchan;	// for overflow protection
	size_t	nbytes_decoded;
};

//////////////////////////////////////////////////////////////////////////////

// guid.c

#undef buf
extern char *guid128_format(
	/*@returned@*/ /*@out@*/ char *const restrict buf,
	const struct Guid128 *const restrict
)
/*@modifies	*buf@*/
;

//--------------------------------------------------------------------------//

// metatags_skip.c

#undef file
extern enum FileCheck metatags_skip(FILE *const restrict file)
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
	struct FileStats *const restrict fstat,	FILE *const restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		fstat,
		file
@*/
;

//--------------------------------------------------------------------------//

// tta_seek.c

#undef st
extern void seektable_init(
	/*@out@*/ struct SeekTable *const restrict st, size_t
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
extern void seektable_add(
	struct SeekTable *const restrict st, size_t, size_t, const char *
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
extern void seektable_free(struct SeekTable *const restrict st)
/*@globals		internalState@*/
/*@modifies		internalState,
			*st
@*/
/*@releases		st->table@*/
/*@ensures isnull	st->table@*/
;

//--------------------------------------------------------------------------//

#undef st
#undef file
extern enum FileCheck filecheck_tta_seektable(
	/*@out@*/ struct SeekTable *const restrict st, size_t,
	FILE *const restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		st,
		fstat,
		file
@*/
;

//--------------------------------------------------------------------------//

// tta_write.c

#undef outfile
extern void prewrite_tta1_header_seektable(
	FILE *const restrict outfile, const struct SeekTable *const restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
extern off_t write_tta1_header(
	FILE *const restrict outfile, size_t,
	const struct FileStats *const restrict, const char *
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
extern void write_tta_seektable(
	FILE *const restrict outfile, const struct SeekTable *const restrict
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
	struct FileStats *const restrict fstat, FILE *const restrict file
)
/*@modifies	fstat,
		file
@*/
;

//--------------------------------------------------------------------------//

// w64_write.c

#undef outfile
extern void prewrite_w64_header(FILE *const restrict outfile)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
extern void write_w64_header(
	FILE *const restrict outfile, size_t,
	const struct FileStats *const restrict
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
	struct FileStats *const restrict fstat, FILE *const restrict file
)
/*@modifies	fstat,
		file
@*/
;

#undef fstat
#undef file
extern enum FileCheck filecheck_wav_read_subchunk_fmt(
	struct FileStats *const restrict fstat, FILE *const restrict file
)
/*@modifies	fstat,
		file
@*/
;

//--------------------------------------------------------------------------//

// wav_write.c

#undef outfile
extern void prewrite_wav_header(FILE *const restrict outfile)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef outfile
extern void write_wav_header(
	FILE *const restrict outfile, size_t,
	const struct FileStats *const restrict, const char *
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
;

#undef body
extern void fill_waveformatex_body(
	struct RiffSubChunk_WaveFormatEX_Body *const restrict body, u16,
	const struct FileStats *const restrict
)
/*@modifies	*body@*/
;

#undef wfx
extern void fill_waveformatextensible(
	struct RiffSubChunk_WaveFormatExtensible_Tail *const restrict wfx,
	const struct FileStats *const restrict
)
/*@modifies	*wfx@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
