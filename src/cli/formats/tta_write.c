//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta_write.c                                                      //
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
	const struct SeekTable *const restrict st
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	union {
		int	d;
		off_t	o;
	} t;

	// header + seektable + st-crc
	t.o = (off_t) (
		  sizeof(struct TTA1Header)
		+ (st->limit * (sizeof *st->table)) + sizeof(u32)
	);
	(void) fflush(outfile);
	t.d = ftruncate(fileno(outfile), t.o);
	if ( t.d != 0 ){
		error_sys(errno, "ftruncate", strerror(errno), NULL);
	}
	(void) fseeko(outfile, 0, SEEK_END);

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
	union {
		size_t z;
	} t;

	(void) memcpy(&h.preamble, TTA1_PREAMBLE, sizeof h.preamble);
	h.format	= htole16(WAVE_FMT_PCM);
	h.nchan		= htole16(stats->nchan);
	h.samplebits	= htole16(stats->samplebits);
	h.samplerate	= htole32(stats->samplerate);
	if ( nsamples_perchan_total > UINT32_MAX ){
		warning_tta("%s: broken header field: nsamples",
			outfile_name
		);
	}
	h.nsamples	= htole32((u32) nsamples_perchan_total);
	h.crc		= htole32(libttaR_crc32(
		(u8 *) &h, (sizeof h) - (sizeof h.crc)
	));

	// write
	t.z = fwrite(&h, sizeof h, (size_t) 1, outfile);
	if ( t.z != (size_t) 1 ){
		error_sys(errno, "fwrite", strerror(errno), outfile_name);
	}

	return ftello(outfile);
}

// TODO fmt mode param
void
write_tta_seektable(
	FILE *const restrict outfile,
	const struct SeekTable *const restrict st
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	u32 crc;

	(void) fseeko(outfile, st->off, SEEK_SET);

	// write seektable
	(void) fwrite(st->table, sizeof *st->table, st->nmemb, outfile);

	// calc then write seektable crc
	crc = libttaR_crc32(
		(u8 *) st->table, st->nmemb * (sizeof *st->table)
	);
	(void) fwrite(&crc, sizeof crc, (size_t) 1, outfile);

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
