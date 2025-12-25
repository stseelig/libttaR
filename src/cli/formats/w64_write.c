/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/w64_write.c                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <unistd.h>

#include "../byteswap.h"
#include "../common.h"
#include "../debug.h"
#include "../formats.h"

#include "./w64.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn prewrite_w64_header
 * @brief reserves space for the Sony Wave64 header
 *
 * @param outfile      - destination file
 * @param outfile_name - name of the destination file (errors)
 *
 * @note MAYBE write a preliminary header instead
**/
BUILD void
prewrite_w64_header(
	FILE *const RESTRICT outfile, const char *const RESTRICT outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	union {	int d; } result;

	result.d = fflush(outfile);
	if UNLIKELY ( result.d != 0 ){
		error_sys(errno, "fflush", outfile_name);
	}

	result.d = ftruncate(
		fileno(outfile),
		(off_t) sizeof(struct Riff64Header_WriteTemplate)
	);
	if UNLIKELY ( (result.d != 0) && (errno != EINVAL) ){ /* /dev/null */
		error_sys(errno, "ftruncate", outfile_name);
	}

	result.d = fseeko(outfile, 0, SEEK_END);
	if UNLIKELY ( result.d != 0 ){
		error_sys(errno, "fseeko", outfile_name);
	}
	return;
}

/**@fn write_w64_header
 * @brief write a Sony Wave64 header
 *
 * @param outfile      - destination file
 * @param data_size    - size of the data chunk
 * @param fstat        - bloated file stats struct
 * @param outfile_name - name of the destination file (errors)
 *
 * @pre outfile should be at correct offset before calling
**/
BUILD void
write_w64_header(
	FILE *const RESTRICT outfile, const size_t data_size,
	const struct FileStats *const RESTRICT fstat,
	const char *const RESTRICT outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	struct Riff64Header_WriteTemplate wt;
	union {	size_t z; } result;

	/* assuming that 64-bit values will not overflow */

	/* riff/wave header */
	(void) memcpy(
		&wt.hdr.rh.guid, &RIFF64_GUID_RIFF, sizeof wt.hdr.rh.guid
	);
	wt.hdr.rh.size	= byteswap_htole_u64(
		(uint64_t) (data_size + (sizeof wt))
	);
	(void) memcpy(
		&wt.hdr.guid, &RIFF64_GUID_WAVE, sizeof wt.hdr.guid
	);

	/* fmt subchunk */
	(void) memcpy(
		&wt.fmt.rh.guid, &RIFF64_GUID_FMT, sizeof wt.fmt.rh.guid
	);
	wt.fmt.rh.size	= byteswap_htole_u64(
		(uint64_t) ((sizeof wt.fmt) + (sizeof wt.wfx))
	);
	fill_waveformatex_body(&wt.fmt.body, WAVE_FMT_EXTENSIBLE, fstat);
	fill_waveformatextensible(&wt.wfx, fstat);

	/* data chunk header */
	(void) memcpy(&wt.data.guid, &RIFF64_GUID_DATA, sizeof wt.data.guid);
	wt.data.size	= byteswap_htole_u64(
		(uint64_t) (data_size + (sizeof wt.data))
	);

	result.z = fwrite(&wt, sizeof wt, SIZE_C(1), outfile);
	if UNLIKELY ( result.z != SIZE_C(1) ){
		error_sys(errno, "fwrite", outfile_name);
	}

	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
