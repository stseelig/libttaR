//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta_seek_check.c                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  http://tausoft.org/wiki/True_Audio_Codec_Format                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../../bits.h"
#include "../../libttaR.h"	// crc32

#include "../alloc.h"
#include "../debug.h"
#include "../formats.h"

//////////////////////////////////////////////////////////////////////////////

/**@fn filecheck_tta_seektable
 * @brief reads a seektable from a TTA file into a seektable struct
 *
 * @param st[out] the seektable struct
 * @param nframes number of frames in the seektable
 * @param file[in] the source file
 *
 * @return FILECHECK_OK if the seektable is good
 *
 * @pre 'file' should be at the appropriate offset before calling
**/
enum FileCheck
filecheck_tta_seektable(
	/*@out@*/ struct SeekTable *const restrict st, const size_t nframes,
	FILE *const restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*st,
		*fstat,
		file
@*/
{
	u32 crc;	// little-endian
	union {	size_t	z;
		int	d;
		u32	u_32;
	} result;

	st->nmemb = nframes;
	st->table = calloc_check(st->nmemb, sizeof *st->table);

	result.z = fread(st->table, sizeof *st->table, st->nmemb, file);
	if ( result.z != st->nmemb ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	result.z = fread(&crc, sizeof crc, (size_t) 1u, file);
	if ( result.z != (size_t) 1u ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	result.u_32 = libttaR_crc32(
		(u8 *) st->table, st->nmemb * (sizeof *st->table)
	);
	if ( result.u_32 != letoh32(crc) ){
		return FILECHECK_CORRUPTED;
	}

	return FILECHECK_OK;
}

// EOF ///////////////////////////////////////////////////////////////////////
