//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta_write.c                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  http://tausoft.org/wiki/True_Audio_Codec_Format                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "../../bits.h"
#include "../../libttaR.h"	// crc32

#include "../debug.h"
#include "../formats.h"

#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

/**@fn prewrite_tta1_header
 * @brief reserves space for the TTA1 header and seektable
 *
 * @param outfile[in] the destination file
 * @param st[in] the seektable
 * @param outfile_name[in] the name of the destination file (errors)
 *
 * @note MAYBE write a preliminary header instead
**/
void
prewrite_tta1_header_seektable(
	FILE *const restrict outfile,
	const struct SeekTable *const restrict st, const char *outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	const off_t offset = (off_t) (	// header + seektable + st-crc
		  sizeof(struct TTA1Header)
		+ (st->limit * (sizeof *st->table)) + sizeof(u32)
	);
	union {	int d; } t;

	t.d = fflush(outfile);
	if UNLIKELY ( t.d != 0 ){
		error_sys(errno, "fflush", outfile_name);
	}

	t.d = ftruncate(fileno(outfile), offset);
	if UNLIKELY ( (t.d != 0) && (errno != EINVAL) ){	// /dev/null
		error_sys(errno, "ftruncate", outfile_name);
	}

	t.d = fseeko(outfile, 0, SEEK_END);
	if UNLIKELY ( t.d != 0 ){
		error_sys(errno, "fseeko", outfile_name);
	}

	return;
}

/**@fn write_tta1_header
 * @brief write a TTA1 header
 *
 * @param outfile[in] the destination file
 * @param nsamples_perchan_total number of samples of 'nchan' channels
 * @param fstat[in] the bloated file stats struct
 * @param outfile_name[in] the name of the destination file (warnings/errors)
 *
 * @return the offset of the end of the header
 *
 * @pre outfile should be at correct offset before calling
**/
off_t
write_tta1_header(
	FILE *const restrict outfile, size_t nsamples_perchan_total,
	const struct FileStats *const restrict fstat, const char *outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	struct TTA1Header h;
	union {	size_t z; } t;

	if UNLIKELY ( nsamples_perchan_total > (size_t) UINT32_MAX ){
		warning_tta("%s: broken header field: nsamples",
			outfile_name
		);
	}

	(void) memcpy(&h.preamble, TTA1_PREAMBLE, sizeof h.preamble);
	h.format	= htole16(WAVE_FMT_PCM);
	h.nchan		= htole16(fstat->nchan);
	h.samplebits	= htole16(fstat->samplebits);
	h.samplerate	= htole32(fstat->samplerate);
	h.nsamples	= htole32(
		nsamples_perchan_total > (size_t) UINT32_MAX
			? UINT32_MAX : (u32) nsamples_perchan_total
	);
	h.crc		= htole32(libttaR_crc32(
		(u8 *) &h, (sizeof h) - (sizeof h.crc)
	));

	// write
	t.z = fwrite(&h, sizeof h, (size_t) 1u, outfile);
	if UNLIKELY ( t.z != (size_t) 1u ){
		error_sys(errno, "fwrite", outfile_name);
	}

	return ftello(outfile);
}

/**@fn write_tta_seektable
 * @brief writes a TTA seektable
 *
 * @param outfile[in] the destination file
 * @param st[in] the seektable struct
 * @param outfile_name[in] the name of the destination file (errors)
**/
void
write_tta_seektable(
	FILE *const restrict outfile,
	const struct SeekTable *const restrict st, const char *outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	u32 crc;
	union {	int	d;
		size_t	z;
	} t;

	t.d = fseeko(outfile, st->off, SEEK_SET);
	if UNLIKELY ( t.d != 0 ){
		error_sys(errno, "fseeko", outfile_name);
	}

	// write seektable
	t.z = fwrite(st->table, sizeof *st->table, st->nmemb, outfile);
	if UNLIKELY ( t.z != st->nmemb ){
		error_sys(errno, "fwrite", outfile_name);
	}

	// calc then write seektable crc
	crc = htole32(libttaR_crc32(
		(u8 *) st->table, st->nmemb * (sizeof *st->table)
	));
	t.z = fwrite(&crc, sizeof crc, (size_t) 1u, outfile);
	if UNLIKELY ( t.z != (size_t) 1u ){
		error_sys(errno, "fwrite", outfile_name);
	}

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
