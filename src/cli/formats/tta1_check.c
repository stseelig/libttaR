//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta1_check.c                                                     //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <string.h>	// memcmp

#include "../../bits.h"
#include "../../libttaR.h"	// crc32

#include "../formats.h"

#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

// returns FILECHECK_OK if file is a TTAv1
// file should be at the appropriate offset before calling
enum FileCheck
filecheck_tta1(
	struct FileStats *const restrict fstat,	FILE *const restrict file
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*fstat,
		file
@*/
{
	struct TTA1Header hdr;
	const off_t start = ftello(file);
	union {	size_t		z;
		int		d;
		u32		u_32;
		enum FileCheck	fc;
	} t;

	t.z = fread(&hdr, sizeof hdr, (size_t) 1u, file);
	if ( t.z != (size_t) 1u ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}
	if ( memcmp(hdr.preamble, TTA1_PREAMBLE, sizeof hdr.preamble) != 0 ){
		// reset file stream and return
		t.d = fseeko(file, start, SEEK_SET);
		if ( t.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	t.u_32 = libttaR_crc32((u8 *) &hdr, (sizeof hdr) - (sizeof hdr.crc));
	if ( t.u_32 != letoh32(hdr.crc) ){
		return FILECHECK_CORRUPTED;
	}

	if ( letoh16(hdr.format) != WAVE_FMT_PCM ){
		return FILECHECK_UNSUPPORTED_DATATYPE;
	}

	fstat->encfmt		= xENCFMT_TTA1;
	fstat->nchan		= letoh16(hdr.nchan);
	fstat->samplebits	= letoh16(hdr.samplebits);
	fstat->samplerate	= letoh32(hdr.samplerate);
	fstat->nsamples_enc	= (size_t) letoh32(hdr.nsamples);

	fstat->enctta_off	= ftello(file);

	return FILECHECK_OK;
}

// EOF ///////////////////////////////////////////////////////////////////////
