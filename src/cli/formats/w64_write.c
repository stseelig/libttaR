//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/w64_write.c                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "../../bits.h"

#include "../debug.h"
#include "../formats.h"	// fill_waveformat*

#include "w64.h"

/////////////////////////////////////////////////////////////////////////////

// MAYBE write preliminary header instead
void
prewrite_w64_header(FILE *const restrict outfile, const char *outfile_name)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	union {	int d; } t;

	t.d = fflush(outfile);
	if UNLIKELY ( t.d != 0 ){
		error_sys(errno, "fflush", outfile_name);
	}

	t.d = ftruncate(
		fileno(outfile),
		(off_t) sizeof(struct Riff64Header_WriteTemplate)
	);
	if UNLIKELY ( (t.d != 0) && (errno != EINVAL) ){	// /dev/null
		error_sys(errno, "ftruncate", outfile_name);
	}

	t.d = fseeko(outfile, 0, SEEK_END);
	if UNLIKELY ( t.d != 0 ){
		error_sys(errno, "fseeko", outfile_name);
	}

	return;
}

// out stream should be at correct position before calling
void
write_w64_header(
	FILE *const restrict outfile, size_t data_size,
	const struct FileStats *const restrict fstat, const char *outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	struct Riff64Header_WriteTemplate wt;
	union {	int z; } t;

	// riff/wave header
	(void) memcpy(
		&wt.hdr.rh.guid, &RIFF64_GUID_RIFF, sizeof wt.hdr.rh.guid
	);
	wt.hdr.rh.size	= htole64(data_size + (sizeof wt));
	(void) memcpy(
		&wt.hdr.guid, &RIFF64_GUID_WAVE, sizeof wt.hdr.guid
	);

	// fmt subchunk
	(void) memcpy(
		&wt.fmt.rh.guid, &RIFF64_GUID_FMT, sizeof wt.fmt.rh.guid
	);
	wt.fmt.rh.size	= htole64((sizeof wt.fmt) + (sizeof wt.wfx));
	fill_waveformatex_body(&wt.fmt.body, WAVE_FMT_EXTENSIBLE, fstat);
	fill_waveformatextensible(&wt.wfx, fstat);

	// data chunk header
	(void) memcpy(&wt.data.guid, &RIFF64_GUID_DATA, sizeof wt.data.guid);
	wt.data.size	= htole64(data_size + (sizeof wt.data));

	t.z = fwrite(&wt, sizeof wt, (size_t) 1u, outfile);
	if UNLIKELY ( t.z != (size_t) 1u ){
		error_sys_nf(errno, "fwrite", outfile_name);
	}

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
