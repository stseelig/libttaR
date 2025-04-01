//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/w64_check.c                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>	// memcmp

#include "../../bits.h"

#include "../formats.h"

#include "w64.h"

//////////////////////////////////////////////////////////////////////////////

#undef file
static enum FileCheck filecheck_w64_find_subchunk(
	FILE *const restrict file, const struct Guid128 *const restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn filecheck_w64
 * @brief checks if a file is a Sony Wave64
 *
 * @param fstat[out] the bloated file stats struct
 * @param file[in] the source file
 *
 * @return FILECHECK_OK if file format is Sony Wave64
 *
 * @pre 'file' should be at the appropriate offset before calling
**/
enum FileCheck
filecheck_w64(
	/*@out@*/ struct FileStats *const restrict fstat,
	FILE *const restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
{
	union { struct Riff64Header		rh;
		struct Riff64ChunkHeader_Wave	wave;
	} chunk;
	const off_t start = ftello(file);
	union {	size_t		z;
		int		d;
		enum FileCheck	fc;
	} result;

	// Riff64 chunk
	result.z = fread(&chunk.wave, sizeof chunk.wave, (size_t) 1u, file);
	if ( result.z != (size_t) 1u ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}
	if ( (memcmp(
		&chunk.wave.rh.guid, &RIFF64_GUID_RIFF,
		sizeof chunk.wave.rh.guid
	     ) != 0)
	    ||
	     (memcmp(
		&chunk.wave.guid, &RIFF64_GUID_WAVE, sizeof chunk.wave.guid
	     ) != 0)
	){
		// reset file stream and return
		result.d = fseeko(file, start, SEEK_SET);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	// search for format subchunk
	result.fc = filecheck_w64_find_subchunk(file, &RIFF64_GUID_FMT);
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

	// search for data subchunk header
	result.fc = filecheck_w64_find_subchunk(file, &RIFF64_GUID_DATA);
	if ( result.fc != FILECHECK_OK ){
		return result.fc;
	}
	result.z = fread(&chunk.rh, sizeof chunk.rh, (size_t) 1u, file);
	if ( result.z != (size_t) 1u ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	fstat->decfmt      = DECFMT_W64;
	fstat->decpcm_off  = ftello(file);

	if ( (size_t) letoh64(chunk.rh.size) <= sizeof chunk.rh ){
		return FILECHECK_MALFORMED;
	}
	fstat->decpcm_size = (size_t) (
		letoh64(chunk.rh.size) - (sizeof chunk.rh)
	);

	return FILECHECK_OK;
}

//--------------------------------------------------------------------------//

/**@fn filecheck_w64_find_subchunk
 * @brief searches for a RIFF64 subchunk
 *
 * @param file[in] the source file
 * @param target[in] ID of the subchunk we are searching for
 *
 * @return FILECHECK_OK if found
 *
 * @pre 'file' should be at the beginning of a subchunk before calling
**/
static enum FileCheck
filecheck_w64_find_subchunk(
	FILE *const restrict file, const struct Guid128 *const restrict target
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
{
	struct Riff64Header rh;
	union {	size_t	z;
		int	d;
	} result;

	goto loop_entr;
	do {
		// seek to end of current subchunk
		result.d = fseeko(
			file, (off_t) (letoh64(rh.size) - (sizeof rh)),
			SEEK_CUR
		);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
loop_entr:
		result.z = fread(&rh, sizeof rh, (size_t) 1u, file);
		if ( result.z != (size_t) 1u ){
			if ( feof(file) != 0 ){
				return FILECHECK_MALFORMED;
			}
			return FILECHECK_READ_ERROR;
		}

		if ( letoh64(rh.size) <= sizeof rh ){
			return FILECHECK_MALFORMED;
		}
	}
	while ( memcmp(&rh.guid, target, sizeof rh.guid) != 0 );

	// seek to start of subchunk before returning
	result.d = fseeko(file, -((off_t) (sizeof rh)), SEEK_CUR);
	if ( result.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	return FILECHECK_OK;
}

// EOF ///////////////////////////////////////////////////////////////////////
