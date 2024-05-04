//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/wav_check.c                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>	// memcmp

#include "../../bits.h"

#include "../formats.h"

#include "wav.h"

//////////////////////////////////////////////////////////////////////////////

#undef file
static enum FileCheck filecheck_wav_find_subchunk(
	FILE *const restrict file, const char *const restrict
)
/*@modifies	file@*/
;

//////////////////////////////////////////////////////////////////////////////

// returns FILECHECK_OK if file is a Microsoft RIFF/WAVE
// file should be at the appropriate offset before calling
enum FileCheck
filecheck_wav(
	struct FileStats *const restrict fstat, FILE *const restrict file
)
/*@modifies	fstat,
		file
@*/
{
	union { struct RiffHeader		rh;
		struct RiffChunkHeader_Wave	wave;
	} chunk;
	const off_t start = ftello(file);
	union {
		size_t		z;
		int		d;
		enum FileCheck	fc;
	} t;

	// RIFF chunk
	t.z = fread(&chunk.wave, sizeof chunk.wave, (size_t) 1, file);
	if ( t.z != (size_t) 1 ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}
	if ( (memcmp(
		chunk.wave.rh.id, RIFF_ID_RIFF, sizeof chunk.wave.rh.id
	     ) != 0)
	    ||
	     (memcmp(
		chunk.wave.format, RIFF_ID_WAVE, sizeof chunk.wave.format
	     ) != 0)
	){
		// reset file stream and return
		t.d = fseeko(file, start, SEEK_SET);
		if ( t.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	// search for format subchunk
	t.fc = filecheck_wav_find_subchunk(file, RIFF_ID_FMT);
	if ( t.fc != FILECHECK_OK ){
		return t.fc;
	}
	t.d = fseeko(file, (off_t) (sizeof chunk.rh), SEEK_CUR);
	if ( t.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	t.fc = filecheck_wav_read_subchunk_fmt(fstat, file);
	if ( t.fc != FILECHECK_OK ){
		return t.fc;
	}

	// search for data subchunk header
	t.fc = filecheck_wav_find_subchunk(file, RIFF_ID_DATA);
	if ( t.fc != FILECHECK_OK ){
		return t.fc;
	}
	t.z = fread(&chunk.rh, sizeof chunk.rh, (size_t) 1, file);
	if ( t.z != (size_t) 1 ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	fstat->decfmt      = FORMAT_WAV;
	fstat->decpcm_off  = ftello(file);
	fstat->decpcm_size = (size_t) letoh32(chunk.rh.size);

	return FILECHECK_OK;
}

// must have stream set past the subchunk header
enum FileCheck
filecheck_wav_read_subchunk_fmt(
	struct FileStats *const restrict fstat, FILE *const restrict file
)
/*@modifies	fstat,
		file
@*/
{
	union {	struct RiffSubChunk_WaveFormatEX_Body		fmt;
		struct RiffSubChunk_WaveFormatExtensible_Tail	wfx;
	} chunk;
	u16 format;
	union {
		size_t		z;
		off_t		o;
		int		d;
	} t;

	t.z = fread(&chunk.fmt, sizeof chunk.fmt, (size_t) 1, file);
	if ( t.z != (size_t) 1 ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	format = letoh16(chunk.fmt.format);
	fstat->wavformat	= format;
	if ( (format !=  WAVE_FMT_PCM) && (format !=  WAVE_FMT_EXTENSIBLE) ){
		return FILECHECK_UNSUPPORTED_DATATYPE;
	}

	fstat->endian		= xENDIAN_LITTLE;
	fstat->nchan		= letoh16(chunk.fmt.nchan);
	fstat->samplebits	= letoh16(chunk.fmt.samplebits);
	fstat->samplerate	= letoh32(chunk.fmt.samplerate);
	if ( fstat->samplebits <= (u16) 8u ){
		fstat->inttype 	= INT_UNSIGNED;
	}
	else {	fstat->inttype	= INT_SIGNED; }

	if ( format == WAVE_FMT_EXTENSIBLE ){
		t.z = fread(&chunk.wfx, sizeof chunk.wfx, (size_t) 1, file);
		if ( t.z != (size_t) 1 ){
			if ( feof(file) != 0 ){
				return FILECHECK_MALFORMED;
			}
			return FILECHECK_READ_ERROR;
		}

		fstat->chanmask_wav = letoh32(chunk.wfx.chanmask);
		(void) memcpy(
			&fstat->wavsubformat, &chunk.wfx.subformat,
			sizeof chunk.wfx.subformat
		);

		t.d = memcmp(
			&chunk.wfx.subformat, &WAVE_SUBFMT_PCM,
			sizeof chunk.wfx.subformat
		);
		if ( t.d != 0 ){
			return FILECHECK_UNSUPPORTED_DATATYPE;
		}

		// seek to end of Extensible
		t.o = (off_t) (
			  (sizeof chunk.wfx)
			- (letoh16(chunk.wfx.size) + (sizeof chunk.wfx.size))
		);
		t.d = fseeko(file, t.o, SEEK_CUR);
		if ( t.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
	}

	return FILECHECK_OK;
}

//--------------------------------------------------------------------------//

static enum FileCheck
filecheck_wav_find_subchunk(
	FILE *const restrict file, const char *const restrict target
)
/*@modifies	file@*/
{
	struct RiffHeader rh;
	union {
		size_t	z;
		int	d;
	} t;

	// check subchunks until target is found
	goto loop_entr;
	do {
		// seek to end of current subchunk
		t.d = fseeko(file, (off_t) rh.size, SEEK_CUR);
		if ( t.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
loop_entr:
		t.z = fread(&rh, sizeof rh, (size_t) 1, file);
		if ( t.z != (size_t) 1 ){
			if ( feof(file) != 0 ){
				return FILECHECK_MALFORMED;
			}
			return FILECHECK_READ_ERROR;
		}
	}
	while ( memcmp(&rh.id, target, sizeof rh.id) != 0 );

	// seek to start of subchunk before returning
	t.d = fseeko(file, -((off_t) (sizeof rh)), SEEK_CUR);
	if ( t.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	return FILECHECK_OK;
}

// EOF ///////////////////////////////////////////////////////////////////////
