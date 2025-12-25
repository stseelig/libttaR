/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/wav_check.c                                                      //
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
#include <string.h>

#include "../byteswap.h"
#include "../common.h"
#include "../formats.h"

#include "./wav.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef file
static enum FileCheck filecheck_wav_find_subchunk(
	FILE *const RESTRICT file, const uint8_t *const RESTRICT
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn filecheck_wav
 * @brief checks if a file is Microsoft RIFF/WAVE
 *
 * @param fstat[out] the bloated file stats struct
 * @param file[in] the source file
 *
 * @return FILECHECK_OK if file format is Microsoft RIFF/WAVE
 *
 * @pre 'file' should be at the appropriate offset before calling
**/
BUILD enum FileCheck
filecheck_wav(
	/*@out@*/ struct FileStats *const RESTRICT fstat,
	FILE *const RESTRICT file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
{
	const off_t start = ftello(file);
	/* * */
	union { struct RiffHeader		rh;
		struct RiffChunkHeader_Wave	wave;
	} chunk;
	union {	size_t		z;
		int		d;
		enum FileCheck	fc;
	} result;

	/* RIFF chunk */
	result.z = fread(&chunk.wave, sizeof chunk.wave, SIZE_C(1), file);
	if ( result.z != SIZE_C(1) ){
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
		/* reset file stream and return */
		result.d = fseeko(file, start, SEEK_SET);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	/* search for format subchunk */
	result.fc = filecheck_wav_find_subchunk(file, RIFF_ID_FMT);
	if ( result.fc != FILECHECK_OK ){
		return result.fc;
	}
	result.d = fseeko(file, (off_t) (sizeof chunk.rh), SEEK_CUR);
	if ( result.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	result.fc = filecheck_wav_read_subchunk_fmt(fstat, file);
	if ( result.fc != FILECHECK_OK ){
		return result.fc;
	}

	/* search for data subchunk header */
	result.fc = filecheck_wav_find_subchunk(file, RIFF_ID_DATA);
	if ( result.fc != FILECHECK_OK ){
		return result.fc;
	}
	result.z = fread(&chunk.rh, sizeof chunk.rh, SIZE_C(1), file);
	if ( result.z != SIZE_C(1) ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	fstat->decfmt      = DECFMT_WAV;
	fstat->decpcm_off  = ftello(file);
	fstat->decpcm_size = (size_t) byteswap_letoh_u32(chunk.rh.size);

	return FILECHECK_OK;
}

/**@fn filecheck_wav_read_subchunk_fmt
 * @brief
 *
 * @param fstat - bloated file stats struct
 * @param file  - source file
 *
 * @return FILECHECK_OK if no errors
 *
 * @pre must have stream set past the subchunk header
**/
BUILD enum FileCheck
filecheck_wav_read_subchunk_fmt(
	/*@out@*/ struct FileStats *const RESTRICT fstat,
	FILE *const RESTRICT file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
{
	union {	struct RiffSubChunk_WaveFormatEX_Body		fmt;
		struct RiffSubChunk_WaveFormatExtensible_Tail	wfx;
	} chunk;
	uint16_t format;
	union {	size_t	z;
		off_t	o;
		int	d;
	} result;

	result.z = fread(&chunk.fmt, sizeof chunk.fmt, SIZE_C(1), file);
	if ( result.z != SIZE_C(1) ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	format = byteswap_letoh_u16(chunk.fmt.format);
	fstat->wavformat	= format;
	if ( (format != WAVE_FMT_PCM) && (format != WAVE_FMT_EXTENSIBLE) ){
		return FILECHECK_UNSUPPORTED_DATATYPE;
	}

	fstat->endian		= xENDIAN_LITTLE;
	fstat->nchan		= byteswap_letoh_u16(chunk.fmt.nchan);
	fstat->samplebits	= byteswap_letoh_u16(chunk.fmt.samplebits);
	fstat->samplerate	= byteswap_letoh_u32(chunk.fmt.samplerate);
	fstat->inttype		= (
		fstat->samplebits <= UINT16_C(8) ? INT_UNSIGNED : INT_SIGNED
	);

	if ( format == WAVE_FMT_EXTENSIBLE ){
		result.z = fread(
			&chunk.wfx, sizeof chunk.wfx, SIZE_C(1), file
		);
		if ( result.z != SIZE_C(1) ){
			if ( feof(file) != 0 ){
				return FILECHECK_MALFORMED;
			}
			return FILECHECK_READ_ERROR;
		}

		fstat->chanmask_wav = byteswap_letoh_u32(chunk.wfx.chanmask);
		(void) memcpy(
			&fstat->wavsubformat, &chunk.wfx.subformat,
			sizeof chunk.wfx.subformat
		);

		result.d = memcmp(
			&chunk.wfx.subformat, &WAVE_SUBFMT_PCM,
			sizeof chunk.wfx.subformat
		);
		if ( result.d != 0 ){
			return FILECHECK_UNSUPPORTED_DATATYPE;
		}

		/* seek to end of Extensible */
		if ( byteswap_letoh_u16(chunk.wfx.size) == 0 ){
			return FILECHECK_MALFORMED;
		}
		result.o = (off_t) (
			  (sizeof chunk.wfx)
			- (byteswap_letoh_u16(chunk.wfx.size)
			+ (sizeof chunk.wfx.size))
		);
		result.d = fseeko(file, result.o, SEEK_CUR);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
	}
	return FILECHECK_OK;
}

/* ------------------------------------------------------------------------ */

/**@fn filecheck_wav_find_subchunk
 * @brief searches for a RIFF subchunk
 *
 * @param file   - source file
 * @param target - ID of the subchunk we are searching for
 *
 * @return FILECHECK_OK if found
 *
 * @pre 'file' should be at the beginning of a subchunk before calling
**/
static enum FileCheck
filecheck_wav_find_subchunk(
	FILE *const RESTRICT file, const uint8_t *const RESTRICT target
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
{
	struct RiffHeader rh;
	union {	size_t	z;
		int	d;
	} result;

	/* check subchunks until target is found */
	goto loop_entr;
	do {
		/* seek to end of current subchunk */
		result.d = fseeko(
			file, (off_t) byteswap_letoh_u32(rh.size), SEEK_CUR
		);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
loop_entr:
		result.z = fread(&rh, sizeof rh, SIZE_C(1), file);
		if ( result.z != SIZE_C(1) ){
			if ( feof(file) != 0 ){
				return FILECHECK_MALFORMED;
			}
			return FILECHECK_READ_ERROR;
		}

		if ( byteswap_letoh_u32(rh.size) == 0 ){
			return FILECHECK_MALFORMED;
		}
	}
	while ( memcmp(&rh.id, target, sizeof rh.id) != 0 );

	/* seek to start of subchunk before returning */
	result.d = fseeko(file, -((off_t) (sizeof rh)), SEEK_CUR);
	if ( result.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	return FILECHECK_OK;
}

/* EOF //////////////////////////////////////////////////////////////////// */
