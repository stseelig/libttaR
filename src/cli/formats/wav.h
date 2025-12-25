#ifndef H_TTA_FORMATS_WAV_H
#define H_TTA_FORMATS_WAV_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/wav.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// http://soundfile.sapp.org/doc/WaveFormat/                                //
// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ksmedia/  //
//  ns-ksmedia-waveformatextensible                                         //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>

#include "../ascii-literals.h"
#include "../common.h"

#include "./guid.h"

/* //////////////////////////////////////////////////////////////////////// */

#define RIFF_ID_RIFF	((uint8_t[4u]) { \
	ASCII_R_UP, ASCII_I_UP, ASCII_F_UP, ASCII_F_UP \
})

#define RIFF_ID_WAVE	((uint8_t[4u]) { \
	ASCII_W_UP, ASCII_A_UP, ASCII_V_UP, ASCII_E_UP \
})

#define RIFF_ID_FMT	((uint8_t[4u]) { \
	ASCII_F_LO, ASCII_M_LO, ASCII_T_LO, ASCII_SP \
})

#define RIFF_ID_DATA	((uint8_t[4u]) { \
	ASCII_D_LO, ASCII_A_LO, ASCII_T_LO, ASCII_A_LO \
})

#define WAVE_FMT_PCM		UINT16_C(0x0001)
#define WAVE_FMT_EXTENSIBLE	UINT16_C(0xFFFE)

#define WAVE_SUBFMT_PCM 	((struct Guid128) { \
	{ UINT8_C(0x01), UINT8_C(0x00), UINT8_C(0x00), UINT8_C(0x00) }, \
	{ UINT8_C(0x00), UINT8_C(0x00) }, \
	{ UINT8_C(0x10), UINT8_C(0x00) }, \
	{ UINT8_C(0x80), UINT8_C(0x00) }, \
	{ UINT8_C(0x00), UINT8_C(0xAA), UINT8_C(0x00), UINT8_C(0x38), \
		UINT8_C(0x9B), UINT8_C(0x71) \
	} \
})

/* differs from TTA2 layout */
#define WAVE_CHAN_FL		UINT32_C(0x00000001)
#define WAVE_CHAN_FR		UINT32_C(0x00000002)
#define WAVE_CHAN_FC		UINT32_C(0x00000004)
#define WAVE_CHAN_LFE		UINT32_C(0x00000008)
#define WAVE_CHAN_BL		UINT32_C(0x00000010)
#define WAVE_CHAN_BR		UINT32_C(0x00000020)
#define WAVE_CHAN_FLC		UINT32_C(0x00000040)
#define WAVE_CHAN_FRC		UINT32_C(0x00000080)
#define WAVE_CHAN_BC		UINT32_C(0x00000100)
#define WAVE_CHAN_SL		UINT32_C(0x00000200)
#define WAVE_CHAN_SR		UINT32_C(0x00000400)
#define WAVE_CHAN_TC		UINT32_C(0x00000800)
#define WAVE_CHAN_TFL		UINT32_C(0x00001000)
#define WAVE_CHAN_TFC		UINT32_C(0x00002000)
#define WAVE_CHAN_TFR		UINT32_C(0x00004000)
#define WAVE_CHAN_TBL		UINT32_C(0x00008000)
#define WAVE_CHAN_TBC		UINT32_C(0x00010000)
#define WAVE_CHAN_TBR		UINT32_C(0x00020000)

#define WAVE_CHAN_NAMED_NMEMB	18u
#define WAVE_CHAN_NAMED_ARRAY	{ \
	"FL","FR","FC","LFE","BL","BR","FLC","FRC","BC","SL","SR","TC", \
	"TFL","TFC","TFR","TBL","TBC","TBR" \
}

/* //////////////////////////////////////////////////////////////////////// */

/* MAYBE: custom TTA2 channel mask chunk */

/* all ints are little-endian */

struct RiffHeader {
	uint8_t		id[4u];
	uint32_t	size;	/* size of the chunk, inclusive (+8u) */
} PACKED;

struct RiffChunkHeader_Wave {
	struct RiffHeader	rh;		/* .id = .ascii "RIFF"     */
	uint8_t			format[4u];	/* .ascii "WAVE"           */
} PACKED;

struct RiffSubChunk_WaveFormatEX_Body {
	uint16_t	format;		/* WAVE_FMT_PCM/EXTENSIBLE */
	uint16_t	nchan;
	uint32_t	samplerate;
	uint32_t	byterate;	/* useless fields needed by ffmpeg */
	uint16_t	blockalign;	/* nchan * samplebytes; ^^^ :^P    */
	uint16_t	samplebits;
} PACKED;

struct RiffSubChunk_WaveFormatExtensible_Tail {
	uint16_t				size;
	union {	uint16_t valid_samplebits;
		uint16_t nsamples_block;
		uint16_t reserved;
	} 					samples;
	uint32_t				chanmask; /* WAVE_CHAN */
	struct Guid128				subformat;
} PACKED;

struct RiffSubChunk_WaveFormatEX {
	struct RiffHeader			rh; /* .id = .ascii "fmt " */
	struct RiffSubChunk_WaveFormatEX_Body	body;
} PACKED;

/* ------------------------------------------------------------------------ */

struct RiffHeader_WriteTemplate {
	struct RiffChunkHeader_Wave			hdr;
	struct RiffSubChunk_WaveFormatEX		fmt;
	struct RiffSubChunk_WaveFormatExtensible_Tail	wfx;
	struct RiffHeader				data;
} PACKED;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_FORMATS_WAV_H */
