/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/wav_write.c                                                      //
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

#include "./wav.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn prewrite_wav_header
 * @brief reserves space for the Microsoft RIFF/WAVE header
 *
 * @param outfile      - destination file
 * @param outfile_name - name of the destination file (errors)
**/
/* MAYBE: write a preliminary header instead */
BUILD void
prewrite_wav_header(
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
		(off_t) sizeof(struct RiffHeader_WriteTemplate)
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

/**@fn write_wav_header
 * @brief write a Microsoft RIFF/WAVE header
 *
 * @param outfile      - destination file
 * @param data_size    - size of the data chunk
 * @param fstat        - bloated file stats struct
 * @param outfile_name - name of the destination file (errors)
 *
 * @pre outfile should be at correct offset before calling
**/
BUILD void
write_wav_header(
	FILE *const RESTRICT outfile, size_t data_size,
	const struct FileStats *const RESTRICT fstat,
	const char *const RESTRICT outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	struct RiffHeader_WriteTemplate wt;
	union {	uint32_t	u_32; } tmp;
	union {	size_t	z;    } result;

	if UNLIKELY ( data_size > (size_t) UINT32_MAX ){
		warning_tta("%s: broken header field: riff-size",
			outfile_name
		);
	}
	if UNLIKELY (
		data_size
	       >
	        (size_t) (UINT32_MAX - (sizeof wt) + (sizeof wt.hdr))
	){
		warning_tta("%s: broken chunk field: data-size",
			outfile_name
		);
	}

	/* riff/wave header */
	(void) memcpy(&wt.hdr.rh.id, RIFF_ID_RIFF, sizeof wt.hdr.rh.id);
	tmp.u_32 = (
		data_size >= (size_t) (
			UINT32_MAX - (sizeof wt) + (sizeof wt.hdr)
		)
			? UINT32_MAX
			: (uint32_t) (
				data_size + (sizeof wt) - (sizeof wt.hdr.rh)
			)
	);
	wt.hdr.rh.size	= byteswap_htole_u32(tmp.u_32);
	(void) memcpy(&wt.hdr.format, RIFF_ID_WAVE, sizeof wt.hdr.format);

	/* fmt subchunk */
	(void) memcpy(&wt.fmt.rh.id, RIFF_ID_FMT, sizeof wt.fmt.rh.id);
	wt.fmt.rh.size	= byteswap_htole_u32((uint32_t) (
		(sizeof wt.fmt) + (sizeof wt.wfx) - (sizeof wt.fmt.rh)
	));
	fill_waveformatex_body(&wt.fmt.body, WAVE_FMT_EXTENSIBLE, fstat);
	fill_waveformatextensible(&wt.wfx, fstat);

	/* data chunk header */
	(void) memcpy(&wt.data.id, RIFF_ID_DATA, sizeof wt.data.id);
	tmp.u_32 = (data_size >= (size_t) UINT32_MAX
		? UINT32_MAX : (uint32_t) data_size
	);
	wt.data.size	= byteswap_htole_u32(tmp.u_32);

	result.z = fwrite(&wt, sizeof wt, SIZE_C(1), outfile);
	if UNLIKELY ( result.z != SIZE_C(1) ){
		error_sys(errno, "fwrite", outfile_name);
	}
	return;
}

/* ======================================================================== */

/**@fn fill_waveformatex_body
 * @brief fills a RiffSubChunk_WaveFormatEX_Body struct
 *
 * @param body   - struct to fill
 * @param format - (WAVE_FMT_PCM || WAVE_FMT_EXTENSIBLE)
 * @param fstat  - bloated file stats struct
**/
BUILD void
fill_waveformatex_body(
	/*@out@*/ struct RiffSubChunk_WaveFormatEX_Body *const RESTRICT body,
	uint16_t format, const struct FileStats *const RESTRICT fstat
)
/*@modifies	*body@*/
{
	body->format		= byteswap_htole_u16(format);
	body->nchan		= byteswap_htole_u16(fstat->nchan);
	body->samplerate	= byteswap_htole_u32(fstat->samplerate);
	body->byterate		= byteswap_htole_u32((uint32_t)
		(fstat->samplerate * fstat->nchan * fstat->samplebytes)
	);
	body->blockalign	= byteswap_htole_u16((uint16_t)
		(fstat->nchan * fstat->samplebytes)
	);
	body->samplebits	= byteswap_htole_u16(fstat->samplebits);

	return;
}

/**@fn fill_waveformatextensible
 * @brief fills a RiffSubChunk_WaveFormatExtensible_Tail struct
 *
 * @param wfx   - struct to fill
 * @param fstat - bloated file stats struct
**/
BUILD void
fill_waveformatextensible(
	/*@out@*/
	struct RiffSubChunk_WaveFormatExtensible_Tail *const RESTRICT wfx,
	const struct FileStats *const RESTRICT fstat
)
/*@modifies	*wfx@*/
{
	wfx->size			= (
		byteswap_htole_u16((sizeof *wfx) - (sizeof wfx->size))
	);
	wfx->samples.valid_samplebits	= (
		byteswap_htole_u16(fstat->samplebits)
	);
	wfx->chanmask			= (
		byteswap_htole_u32(fstat->chanmask_wav)
	);
	(void) memcpy(
		&wfx->subformat, &WAVE_SUBFMT_PCM, sizeof wfx->subformat
	);

	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
