/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta_write.c                                                      //
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

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "../../libttaR.h"

#include "../byteswap.h"
#include "../common.h"
#include "../debug.h"
#include "../formats.h"

#include "./tta.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn prewrite_tta1_header
 * @brief reserves space for the TTA1 header and seektable
 *
 * @param outfile      - destination file
 * @param st           - seektable
 * @param outfile_name - name of the destination file (errors)
 *
 * @note MAYBE write a preliminary header instead
**/
BUILD void
prewrite_tta1_header_seektable(
	FILE *const RESTRICT outfile,
	const struct SeekTable *const RESTRICT st,
	const char *const RESTRICT outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	const off_t offset = (off_t) (	/* header + seektable + st-crc */
		  sizeof(struct TTA1Header)
		+ (st->limit * (sizeof *st->table)) + sizeof(uint32_t)
	);
	/* * */
	union {	int d; } result;

	result.d = fflush(outfile);
	if UNLIKELY ( result.d != 0 ){
		error_sys(errno, "fflush", outfile_name);
	}

	result.d = ftruncate(fileno(outfile), offset);
	if UNLIKELY ( (result.d != 0) && (errno != EINVAL) ){ /* /dev/null */
		error_sys(errno, "ftruncate", outfile_name);
	}

	result.d = fseeko(outfile, 0, SEEK_END);
	if UNLIKELY ( result.d != 0 ){
		error_sys(errno, "fseeko", outfile_name);
	}

	return;
}

/**@fn write_tta1_header
 * @brief write a TTA1 header
 *
 * @param outfile      - destination file
 * @param nsamples_perchan_total - number of samples of 'nchan' channels
 * @param fstat        - bloated file stats struct
 * @param outfile_name - name of the destination file (warnings/errors)
 *
 * @return the offset of the end of the header
 *
 * @pre outfile should be at correct offset before calling
**/
BUILD off_t
write_tta1_header(
	FILE *const RESTRICT outfile, const size_t nsamples_perchan_total,
	const struct FileStats *const RESTRICT fstat, const char *outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	struct TTA1Header hdr;
	union {	size_t z; } result;

	if UNLIKELY ( nsamples_perchan_total > (size_t) UINT32_MAX ){
		warning_tta("%s: broken header field: nsamples",
			outfile_name
		);
	}

	(void) memcpy(&hdr.preamble, TTA1_PREAMBLE, sizeof hdr.preamble);
	hdr.format	= byteswap_htole_u16(WAVE_FMT_PCM);
	hdr.nchan	= byteswap_htole_u16(fstat->nchan);
	hdr.samplebits	= byteswap_htole_u16(fstat->samplebits);
	hdr.samplerate	= byteswap_htole_u32(fstat->samplerate);
	hdr.nsamples	= byteswap_htole_u32(
		nsamples_perchan_total > (size_t) UINT32_MAX
			? UINT32_MAX : (uint32_t) nsamples_perchan_total
	);
	hdr.crc		= byteswap_htole_u32(libttaR_crc32(
		&hdr, (sizeof hdr) - (sizeof hdr.crc)
	));

	/* write */
	result.z = fwrite(&hdr, sizeof hdr, SIZE_C(1), outfile);
	if UNLIKELY ( result.z != SIZE_C(1) ){
		error_sys(errno, "fwrite", outfile_name);
	}

	return ftello(outfile);
}

/**@fn write_tta_seektable
 * @brief writes a TTA seektable
 *
 * @param outfile      - destination file
 * @param st           - seektable struct
 * @param outfile_name - name of the destination file (errors)
**/
BUILD void
write_tta_seektable(
	FILE *const RESTRICT outfile,
	const struct SeekTable *const RESTRICT st,
	const char *RESTRICT outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	uint32_t crc;	/* little-endian */
	union {	int	d;
		size_t	z;
	} result;

	result.d = fseeko(outfile, st->off, SEEK_SET);
	if UNLIKELY ( result.d != 0 ){
		error_sys(errno, "fseeko", outfile_name);
	}

	/* write seektable */
	result.z = fwrite(st->table, sizeof *st->table, st->nmemb, outfile);
	if UNLIKELY ( result.z != st->nmemb ){
		error_sys(errno, "fwrite", outfile_name);
	}

	/* calc then write seektable CRC */
	crc = byteswap_htole_u32(libttaR_crc32(
		st->table, st->nmemb * (sizeof *st->table)
	));
	result.z = fwrite(&crc, sizeof crc, SIZE_C(1), outfile);
	if UNLIKELY ( result.z != SIZE_C(1) ){
		error_sys(errno, "fwrite", outfile_name);
	}
	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
