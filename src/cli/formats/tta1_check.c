/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta1_check.c                                                     //
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

#include "../../libttaR.h"

#include "../byteswap.h"
#include "../common.h"
#include "../formats.h"

#include "./tta.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn filecheck_tta1
 * @brief check if a file is TTA1
 *
 * @param fstat - bloated file stats struct
 * @param file  - input file
 *
 * @return FILECHECK_OK if file format is TTA1
 *
 * @pre 'file' should be at the appropriate offset before calling
**/
BUILD enum FileCheck
filecheck_tta1(
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
	struct TTA1Header hdr;
	union {	size_t		z;
		int		d;
		uint32_t	u_32;
		enum FileCheck	fc;
	} result;

	result.z = fread(&hdr, sizeof hdr, SIZE_C(1), file);
	if ( result.z != SIZE_C(1) ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}
	if ( memcmp(hdr.preamble, TTA1_PREAMBLE, sizeof hdr.preamble) != 0 ){
		/* reset file stream and return */
		result.d = fseeko(file, start, SEEK_SET);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	result.u_32 = libttaR_crc32(&hdr, (sizeof hdr) - (sizeof hdr.crc));
	if ( result.u_32 != byteswap_letoh_u32(hdr.crc) ){
		return FILECHECK_CORRUPTED;
	}

	if ( byteswap_letoh_u16(hdr.format) != WAVE_FMT_PCM ){
		return FILECHECK_UNSUPPORTED_DATATYPE;
	}

	fstat->encfmt		= xENCFMT_TTA1;
	fstat->nchan		= byteswap_letoh_u16(hdr.nchan);
	fstat->samplebits	= byteswap_letoh_u16(hdr.samplebits);
	fstat->samplerate	= byteswap_letoh_u32(hdr.samplerate);
	fstat->nsamples_enc	= (size_t) byteswap_letoh_u32(hdr.nsamples);

	fstat->enctta_off	= ftello(file);

	return FILECHECK_OK;
}

/* EOF //////////////////////////////////////////////////////////////////// */
