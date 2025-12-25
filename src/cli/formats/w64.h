#ifndef H_TTA_FORMATS_W64_H
#define H_TTA_FORMATS_W64_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/w64.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// https://www.ambisonia.com/Members/mlesse/sony_wave64.pdf/sony_wave64.pdf //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>

#include "../ascii-literals.h"
#include "../common.h"

#include "./guid.h"
#include "./wav.h"

/* //////////////////////////////////////////////////////////////////////// */

#define RIFF64_GUID_RIFF	((struct Guid128) { \
	{ ASCII_R_LO, ASCII_I_LO, ASCII_F_LO, ASCII_F_LO }, \
	{ UINT8_C(0x2E), UINT8_C(0x91) }, \
	{ UINT8_C(0xCF), UINT8_C(0x11) }, \
	{ UINT8_C(0xA5), UINT8_C(0xD6) }, \
	{ UINT8_C(0x28), UINT8_C(0xDB), UINT8_C(0x04), UINT8_C(0xC1), \
		UINT8_C(0x00), UINT8_C(0x00) \
	} \
})

#define RIFF64_GUID_WAVE	((struct Guid128) { \
	{ ASCII_W_LO, ASCII_A_LO, ASCII_V_LO, ASCII_E_LO }, \
	{ UINT8_C(0xF3), UINT8_C(0xAC) }, \
	{ UINT8_C(0xD3), UINT8_C(0x11) }, \
	{ UINT8_C(0x8C), UINT8_C(0xD1) }, \
	{ UINT8_C(0x00), UINT8_C(0xC0), UINT8_C(0x4F), UINT8_C(0x8E), \
		UINT8_C(0xDB), UINT8_C(0x8A) \
	} \
})

#define RIFF64_GUID_FMT		((struct Guid128) { \
	{ ASCII_F_LO, ASCII_M_LO, ASCII_T_LO, ASCII_SP }, \
	{ UINT8_C(0xF3), UINT8_C(0xAC) }, \
	{ UINT8_C(0xD3), UINT8_C(0x11) }, \
	{ UINT8_C(0x8C), UINT8_C(0xD1) }, \
	{ UINT8_C(0x00), UINT8_C(0xC0), UINT8_C(0x4F), UINT8_C(0x8E), \
		UINT8_C(0xDB), UINT8_C(0x8A) \
	} \
})

#define RIFF64_GUID_DATA	((struct Guid128) { \
	{ ASCII_D_LO, ASCII_A_LO, ASCII_T_LO, ASCII_A_LO }, \
	{ UINT8_C(0xF3), UINT8_C(0xAC) }, \
	{ UINT8_C(0xD3), UINT8_C(0x11) }, \
	{ UINT8_C(0x8C), UINT8_C(0xD1) }, \
	{ UINT8_C(0x00), UINT8_C(0xC0), UINT8_C(0x4F), UINT8_C(0x8E), \
		UINT8_C(0xDB), UINT8_C(0x8A) \
	} \
})

/* //////////////////////////////////////////////////////////////////////// */

struct Riff64Header {
	struct Guid128	guid;
	uint64_t	size;	/* !!! inclusive unlike 32-bit WAVE */
} PACKED;

struct Riff64ChunkHeader_Wave {
	struct Riff64Header	rh;
	struct Guid128		guid;
} PACKED;

struct Riff64SubChunk_WaveFormatEX {
	struct Riff64Header			rh;	/* .guid = "fmt " */
	struct RiffSubChunk_WaveFormatEX_Body	body;
} PACKED;

/* ------------------------------------------------------------------------ */

struct Riff64Header_WriteTemplate {
	struct Riff64ChunkHeader_Wave			hdr;
	struct Riff64SubChunk_WaveFormatEX		fmt;
	struct RiffSubChunk_WaveFormatExtensible_Tail	wfx;
	struct Riff64Header				data;
} PACKED;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_FORMATS_W64_H */
