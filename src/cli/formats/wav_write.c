//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/wav_write.c                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
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

#include "wav.h"

//////////////////////////////////////////////////////////////////////////////

/**@fn prewrite_wav_header
 * @brief reserves space for the Microsoft RIFF/WAVE header
 *
 * @param outfile[in] the destination file
 * @param outfile_name[in] the name of the destination file (errors)
 *
 * @note MAYBE write a preliminary header instead
**/
void
prewrite_wav_header(
	FILE *const restrict outfile, const char *const restrict outfile_name
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
	if UNLIKELY ( (result.d != 0) && (errno != EINVAL) ){	// /dev/null
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
 * @param outfile[in] the destination file
 * @param data_size size of the data chunk
 * @param fstat[in] the bloated file stats struct
 * @param outfile_name[in] the name of the destination file (errors)
 *
 * @pre outfile should be at correct offset before calling
**/
void
write_wav_header(
	FILE *const restrict outfile, size_t data_size,
	const struct FileStats *const restrict fstat,
	const char *const restrict outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	struct RiffHeader_WriteTemplate wt;
	union {	u32	u_32; } tmp;
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

	// riff/wave header
	(void) memcpy(&wt.hdr.rh.id, RIFF_ID_RIFF, sizeof wt.hdr.rh.id);
	tmp.u_32 = (
		data_size >= (size_t) (
			UINT32_MAX - (sizeof wt) + (sizeof wt.hdr)
		)
			? UINT32_MAX
			: (u32) (data_size + (sizeof wt) - (sizeof wt.hdr.rh))
	);
	wt.hdr.rh.size	= htole32(tmp.u_32);
	(void) memcpy(&wt.hdr.format, RIFF_ID_WAVE, sizeof wt.hdr.format);

	// fmt subchunk
	(void) memcpy(&wt.fmt.rh.id, RIFF_ID_FMT, sizeof wt.fmt.rh.id);
	wt.fmt.rh.size	= htole32(
		(sizeof wt.fmt) + (sizeof wt.wfx) - (sizeof wt.fmt.rh)
	);
	fill_waveformatex_body(&wt.fmt.body, WAVE_FMT_EXTENSIBLE, fstat);
	fill_waveformatextensible(&wt.wfx, fstat);

	// data chunk header
	(void) memcpy(&wt.data.id, RIFF_ID_DATA, sizeof wt.data.id);
	tmp.u_32 = (data_size >= (size_t) UINT32_MAX
		? UINT32_MAX : (u32) data_size
	);
	wt.data.size	= htole32(tmp.u_32);

	result.z = fwrite(&wt, sizeof wt, (size_t) 1u, outfile);
	if UNLIKELY ( result.z != (size_t) 1u ){
		error_sys(errno, "fwrite", outfile_name);
	}

	return;
}

//==========================================================================//

/**@fn fill_waveformatex_body
 * @brief fills a RiffSubChunk_WaveFormatEX_Body struct
 *
 * @param body[out] struct to fill
 * @param format WAVE_FMT_PCM or WAVE_FMT_EXTENSIBLE
 * @param fstat[in] the bloated file stats struct
**/
void
fill_waveformatex_body(
	/*@out@*/ struct RiffSubChunk_WaveFormatEX_Body *const restrict body,
	u16 format, const struct FileStats *const restrict fstat
)
/*@modifies	*body@*/
{
	body->format		= htole16(format);
	body->nchan		= htole16(fstat->nchan);
	body->samplerate	= htole32(fstat->samplerate);
	body->byterate		= htole32((u32)
		(fstat->samplerate * fstat->nchan * fstat->samplebytes)
	);
	body->blockalign	= htole16((u16)
		(fstat->nchan * fstat->samplebytes)
	);
	body->samplebits	= htole16(fstat->samplebits);
	return;
}

/**@fn fill_waveformatextensible
 * @brief fills a RiffSubChunk_WaveFormatExtensible_Tail struct
 *
 * @param wfx[out] struct to fill
 * @param fstat[in] the bloated file stats struct
**/
void
fill_waveformatextensible(
	/*@out@*/
	struct RiffSubChunk_WaveFormatExtensible_Tail *const restrict wfx,
	const struct FileStats *const restrict fstat
)
/*@modifies	*wfx@*/
{
	wfx->size			= htole16(
		(sizeof *wfx) - (sizeof wfx->size)
	);
	wfx->samples.valid_samplebits	= htole16(fstat->samplebits);
	wfx->chanmask			= htole32(fstat->chanmask_wav);
	(void) memcpy(
		&wfx->subformat, &WAVE_SUBFMT_PCM, sizeof wfx->subformat
	);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
