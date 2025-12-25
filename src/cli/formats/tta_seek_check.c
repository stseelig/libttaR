/* ///////////////////////////////////////////////////////////////////////////
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
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../../libttaR.h"

#include "../alloc.h"
#include "../byteswap.h"
#include "../common.h"
#include "../debug.h"
#include "../formats.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn filecheck_tta_seektable
 * @brief reads a seektable from a TTA file into a seektable struct
 *
 * @param st      - seektable struct
 * @param nframes - number of frames in the seektable
 * @param file    - source file
 *
 * @return FILECHECK_OK if the seektable is good
 *
 * @pre 'file' should be at the appropriate offset before calling
**/
BUILD enum FileCheck
filecheck_tta_seektable(
	/*@out@*/ struct SeekTable *const RESTRICT st, const size_t nframes,
	FILE *const RESTRICT file
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
{
	uint32_t crc;	/* little-endian */
	union {	size_t		z;
		int		d;
		uint32_t	u_32;
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

	result.z = fread(&crc, sizeof crc, SIZE_C(1), file);
	if ( result.z != SIZE_C(1) ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	result.u_32 = libttaR_crc32(
		st->table, st->nmemb * (sizeof *st->table)
	);
	if ( result.u_32 != byteswap_letoh_u32(crc) ){
		return FILECHECK_CORRUPTED;
	}
	return FILECHECK_OK;
}

/* EOF //////////////////////////////////////////////////////////////////// */
