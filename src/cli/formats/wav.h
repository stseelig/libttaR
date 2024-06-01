#ifndef TTA_FORMATS_WAV_H
#define TTA_FORMATS_WAV_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/wav.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// http://soundfile.sapp.org/doc/WaveFormat/                                //
// https://learn.microsoft.com/en-us/windows-hardware/drivers/ddi/ksmedia/  //
//  ns-ksmedia-waveformatextensible                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../../bits.h"

#include "guid.h"

//////////////////////////////////////////////////////////////////////////////

#define RIFF_ID_RIFF	((char[]) {'R','I','F','F'})
#define RIFF_ID_WAVE	((char[]) {'W','A','V','E'})
#define RIFF_ID_FMT	((char[]) {'f','m','t',' '})
#define RIFF_ID_DATA	((char[]) {'d','a','t','a'})

#define WAVE_FMT_PCM		((u16) 0x0001u)
#define WAVE_FMT_EXTENSIBLE	((u16) 0xFFFEu)

#define WAVE_SUBFMT_PCM 	((struct Guid128) { \
{(u8) 0x01u, (u8) 0x00u, (u8) 0x00u, (u8) 0x00u}, \
{(u8) 0x00u, (u8) 0x00u}, \
{(u8) 0x10u, (u8) 0x00u}, \
{(u8) 0x80u, (u8) 0x00u}, \
{(u8) 0x00u, (u8) 0xAAu, (u8) 0x00u, (u8) 0x38u, (u8) 0x9Bu, (u8) 0x71u} \
})

// differs from TTA layout
#define WAVE_CHAN_FL		((u32) 0x00000001u)
#define WAVE_CHAN_FR		((u32) 0x00000002u)
#define WAVE_CHAN_FC		((u32) 0x00000004u)
#define WAVE_CHAN_LFE		((u32) 0x00000008u)
#define WAVE_CHAN_BL		((u32) 0x00000010u)
#define WAVE_CHAN_BR		((u32) 0x00000020u)
#define WAVE_CHAN_FLC		((u32) 0x00000040u)
#define WAVE_CHAN_FRC		((u32) 0x00000080u)
#define WAVE_CHAN_BC		((u32) 0x00000100u)
#define WAVE_CHAN_SL		((u32) 0x00000200u)
#define WAVE_CHAN_SR		((u32) 0x00000400u)
#define WAVE_CHAN_TC		((u32) 0x00000800u)
#define WAVE_CHAN_TFL		((u32) 0x00001000u)
#define WAVE_CHAN_TFC		((u32) 0x00002000u)
#define WAVE_CHAN_TFR		((u32) 0x00004000u)
#define WAVE_CHAN_TBL		((u32) 0x00008000u)
#define WAVE_CHAN_TBC		((u32) 0x00010000u)
#define WAVE_CHAN_TBR		((u32) 0x00020000u)

#define NUM_WAVE_CHAN_NAMED	((uint) 18u)
#define WAVE_CHAN_NAMED_ARRAY	{ \
	"FL","FR","FC","LFE","BL","BR","FLC","FRC","BC","SL","SR","TC", \
	"TFL","TFC","TFR","TBL","TBC","TBR" \
}

//////////////////////////////////////////////////////////////////////////////

// TODO TTA channel mask chunk

// all ints are little-endian

struct RiffHeader {
	char	id[4u];
	u32	size;	// size of the chunk, not including the header(8)
} PACKED;

struct RiffChunkHeader_Wave {
	struct RiffHeader	rh;		// .id = .ascii "RIFF"
	char			format[4u];	// .ascii "WAVE"
} PACKED;

struct RiffSubChunk_WaveFormatEX_Body {
	u16	format;		// WAVE_FMT_PCM/EXTENSIBLE
	u16	nchan;
	u32	samplerate;
	u32	byterate;	// hurray for useless fields needed by ffmpeg
	u16	blockalign;	// nchan * samplebytes; ^^^ :^P
	u16	samplebits;
} PACKED;

struct RiffSubChunk_WaveFormatExtensible_Tail {
	u16				size;
	union {	u16 valid_samplebits;
		u16 nsamples_block;
		u16 reserved;
	} 				samples;
	u32				chanmask;	// WAVE_CHAN
	struct Guid128			subformat;
} PACKED;

struct RiffSubChunk_WaveFormatEX {
	struct RiffHeader			rh;	// .id = .ascii "fmt "
	struct RiffSubChunk_WaveFormatEX_Body	body;
} PACKED;

//--------------------------------------------------------------------------//

struct RiffHeader_WriteTemplate {
	struct RiffChunkHeader_Wave			hdr;
	struct RiffSubChunk_WaveFormatEX		fmt;
	struct RiffSubChunk_WaveFormatExtensible_Tail	wfx;
	struct RiffHeader				data;
} PACKED;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
