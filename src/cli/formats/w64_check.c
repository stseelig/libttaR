//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/w64_check.c                                                      //
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

#include "w64.h"

//////////////////////////////////////////////////////////////////////////////

#undef file
static enum FileCheck filecheck_w64_find_subchunk(
	FILE *const restrict file, const struct Guid128 *const restrict
)
/*@modifies	file@*/
;

//////////////////////////////////////////////////////////////////////////////

// returns FILECHECK_OK if file is a Sony Wave64
// file should be at the appropriate offset before calling
enum FileCheck
filecheck_w64(
	struct FileStats *const restrict fstat, FILE *const restrict file
)
/*@modifies	*fstat,
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
	} t;

	// Riff64 chunk
	t.z = fread(&chunk.wave, sizeof chunk.wave, (size_t) 1u, file);
	if ( t.z != (size_t) 1u ){
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
		t.d = fseeko(file, start, SEEK_SET);
		if ( t.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	// search for format subchunk
	t.fc = filecheck_w64_find_subchunk(file, &RIFF64_GUID_FMT);
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
	t.fc = filecheck_w64_find_subchunk(file, &RIFF64_GUID_DATA);
	if ( t.fc != FILECHECK_OK ){
		return t.fc;
	}
	t.z = fread(&chunk.rh, sizeof chunk.rh, (size_t) 1u, file);
	if ( t.z != (size_t) 1u ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	fstat->decfmt      = DECFMT_W64;
	fstat->decpcm_off  = ftello(file);

	if ( (size_t) letoh64(chunk.rh.size) <= sizeof(struct Riff64Header) ){
		return FILECHECK_MALFORMED;
	}
	fstat->decpcm_size = (size_t) (
		letoh64(chunk.rh.size) - sizeof(struct Riff64Header)
	);

	return FILECHECK_OK;
}

//--------------------------------------------------------------------------//

static enum FileCheck
filecheck_w64_find_subchunk(
	FILE *const restrict file, const struct Guid128 *const restrict target
)
/*@modifies	file@*/
{
	struct Riff64Header rh;
	union {	size_t	z;
		int	d;
	} t;

	goto loop_entr;
	do {
		// seek to end of current subchunk
		t.d = fseeko(file, (off_t) letoh64(rh.size), SEEK_CUR);
		if ( t.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
loop_entr:
		t.z = fread(&rh, sizeof rh, (size_t) 1u, file);
		if ( t.z != (size_t) 1u ){
			if ( feof(file) != 0 ){
				return FILECHECK_MALFORMED;
			}
			return FILECHECK_READ_ERROR;
		}

		if ( letoh64(rh.size) <= sizeof(struct Riff64Header) ){
			return FILECHECK_MALFORMED;
		}
	}
	while ( memcmp(&rh.guid, target, sizeof rh.guid) != 0 );

	// seek to start of subchunk before returning
	t.d = fseeko(file, -((off_t) (sizeof rh)), SEEK_CUR);
	if ( t.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	return FILECHECK_OK;
}

// EOF ///////////////////////////////////////////////////////////////////////
