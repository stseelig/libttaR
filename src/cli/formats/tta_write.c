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

// MAYBE write preliminary header instead
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

// returns the offset of the end of the header
// out stream should be at correct position before calling
off_t
write_tta1_header(
	FILE *const restrict outfile, size_t nsamples_perchan_total,
	const struct FileStats *const restrict stats, const char *outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	struct TTA1Header h;
	union {	size_t z; } t;

	(void) memcpy(&h.preamble, TTA1_PREAMBLE, sizeof h.preamble);
	h.format	= htole16(WAVE_FMT_PCM);
	h.nchan		= htole16(stats->nchan);
	h.samplebits	= htole16(stats->samplebits);
	h.samplerate	= htole32(stats->samplerate);
	if UNLIKELY ( nsamples_perchan_total > (size_t) UINT32_MAX ){
		warning_tta("%s: broken header field: nsamples",
			outfile_name
		);
	}
	h.nsamples	= htole32((u32) nsamples_perchan_total);
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

// TODO fmt mode param
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
	crc = libttaR_crc32(
		(u8 *) st->table, st->nmemb * (sizeof *st->table)
	);
	t.z = fwrite(&crc, sizeof crc, (size_t) 1u, outfile);
	if UNLIKELY ( t.z != (size_t) 1u ){
		error_sys(errno, "fwrite", outfile_name);
	}

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
