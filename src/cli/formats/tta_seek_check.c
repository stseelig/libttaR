//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta_seek_check.c                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
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

#include "../debug.h"
#include "../formats.h"

//////////////////////////////////////////////////////////////////////////////

// returns FILECHECK_OK if the seektable is good
// file should be at the appropriate offset before calling
enum FileCheck
filecheck_tta_seektable(
	/*@out@*/ struct SeekTable *const restrict st, size_t nframes,
	FILE *const restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		st,
		fstat,
		file
@*/
{
	u32 crc;
	union {
		size_t	z;
		int	d;
		u32	u32;
	} t;

	st->nmemb = nframes;
	st->table = calloc(st->nmemb, sizeof *st->table);
	if ( st->table == NULL ){
		error_sys(errno, "calloc", strerror(errno), NULL);
	}
	assert(st->table != NULL);

	t.z = fread(st->table, sizeof *st->table, st->nmemb, file);
	if ( t.z != st->nmemb ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	t.z = fread(&crc, sizeof crc, (size_t) 1, file);
	if ( t.z != (size_t) 1 ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	t.u32 = libttaR_crc32(
		(u8 *) st->table, st->nmemb * (sizeof *st->table)
	);
	if ( t.u32 != letoh32(crc) ){
		return FILECHECK_CORRUPTED;
	}

	return FILECHECK_OK;
}

// EOF ///////////////////////////////////////////////////////////////////////
