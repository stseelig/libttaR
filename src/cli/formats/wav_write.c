//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/wav_write.c                                                      //
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

#include "wav.h"

//////////////////////////////////////////////////////////////////////////////

// MAYBE write preliminary header instead
void
prewrite_wav_header(FILE *const restrict outfile, const char *outfile_name)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	union {
		int	d;
	} t;

	t.d = fflush(outfile);
	if ( t.d != 0 ){
		error_sys_nf(errno, "fflush", strerror(errno), outfile_name);
	}

	t.d = ftruncate(
		fileno(outfile),
		(off_t) sizeof(struct RiffHeader_WriteTemplate)
	);
	if ( t.d != 0 ){
		error_sys_nf(
			errno, "ftruncate", strerror(errno), outfile_name
		);
	}

	t.d = fseeko(outfile, 0, SEEK_END);
	if ( t.d != 0 ){
		error_sys_nf(errno, "fseeko", strerror(errno), outfile_name);
	}

	return;
}

// out stream should be at correct position before calling
void
write_wav_header(
	FILE *const restrict outfile, size_t data_size,
	const struct FileStats *const restrict fstat, const char *outfile_name
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		outfile
@*/
{
	struct RiffHeader_WriteTemplate wt;
	union {
		u32	u_32;
		size_t	z;
	} t;

	if ( data_size > UINT32_MAX ){
		data_size = UINT32_MAX;
		warning_tta("%s: broken header field: size", outfile_name);
	}

	// riff/wave header
	(void) memcpy(&wt.hdr.rh.id, RIFF_ID_RIFF, sizeof wt.hdr.rh.id);
	t.u_32 = (data_size >= (UINT32_MAX - (sizeof wt) + (sizeof wt.hdr))
		? UINT32_MAX
		: data_size + (sizeof wt) - (sizeof wt.hdr.rh)
	);
	wt.hdr.rh.size	= htole32(t.u_32);
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
	t.u_32 = (data_size >= UINT32_MAX ? UINT32_MAX : data_size);
	wt.data.size	= htole32(t.u_32);

	t.z = fwrite(&wt, sizeof wt, (size_t) 1, outfile);
	if ( t.z != (size_t) 1 ){
		error_sys_nf(errno, "fwrite", strerror(errno), outfile_name);
	}

	return;
}

//==========================================================================//

void
fill_waveformatex_body(
	struct RiffSubChunk_WaveFormatEX_Body *const restrict body,
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

void
fill_waveformatextensible(
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
