#ifndef TTA_FORMATS_W64_H
#define TTA_FORMATS_W64_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/w64.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// https://www.ambisonia.com/Members/mlesse/sony_wave64.pdf/sony_wave64.pdf //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../../bits.h"

#include "guid.h"
#include "wav.h"

//////////////////////////////////////////////////////////////////////////////

#define RIFF64_GUID_RIFF	((struct Guid128) { \
{(u8) 'r'  , (u8) 'i'  , (u8) 'f'  , (u8) 'f'}, \
{(u8) 0x2Eu, (u8) 0x91u}, \
{(u8) 0xCFu, (u8) 0x11u}, \
{(u8) 0xA5u, (u8) 0xD6u}, \
{(u8) 0x28u, (u8) 0xDBu, (u8) 0x04u, (u8) 0xC1u, (u8) 0x00u, (u8) 0x00u} \
})

#define RIFF64_GUID_WAVE	((struct Guid128) { \
{(u8) 'w'  , (u8) 'a'  , (u8) 'v'  , (u8) 'e'}, \
{(u8) 0xF3u, (u8) 0xACu}, \
{(u8) 0xD3u, (u8) 0x11u}, \
{(u8) 0x8Cu, (u8) 0xD1u}, \
{(u8) 0x00u, (u8) 0xC0u, (u8) 0x4Fu, (u8) 0x8Eu, (u8) 0xDBu, (u8) 0x8Au} \
})

#define RIFF64_GUID_FMT		((struct Guid128) { \
{(u8) 'f'  , (u8) 'm'  , (u8) 't'  , (u8) ' '}, \
{(u8) 0xF3u, (u8) 0xACu}, \
{(u8) 0xD3u, (u8) 0x11u}, \
{(u8) 0x8Cu, (u8) 0xD1u}, \
{(u8) 0x00u, (u8) 0xC0u, (u8) 0x4Fu, (u8) 0x8Eu, (u8) 0xDBu, (u8) 0x8Au} \
})

#define RIFF64_GUID_DATA	((struct Guid128) { \
{(u8) 'd'  , (u8) 'a'  , (u8) 't'  , (u8) 'a'}, \
{(u8) 0xF3u, (u8) 0xACu}, \
{(u8) 0xD3u, (u8) 0x11u}, \
{(u8) 0x8Cu, (u8) 0xD1u}, \
{(u8) 0x00u, (u8) 0xC0u, (u8) 0x4Fu, (u8) 0x8Eu, (u8) 0xDBu, (u8) 0x8Au} \
})

//////////////////////////////////////////////////////////////////////////////

struct Riff64Header {
	struct Guid128	guid;
	u64		size;	// inclusive unlike 32-bit WAVE
} PACKED;

struct Riff64ChunkHeader_Wave {
	struct Riff64Header	rh;
	struct Guid128		guid;
} PACKED;

struct Riff64SubChunk_WaveFormatEX {
	struct Riff64Header			rh;	// .guid = "fmt "
	struct RiffSubChunk_WaveFormatEX_Body	body;
} PACKED;

//--------------------------------------------------------------------------//

struct Riff64Header_WriteTemplate {
	struct Riff64ChunkHeader_Wave			hdr;
	struct Riff64SubChunk_WaveFormatEX		fmt;
	struct RiffSubChunk_WaveFormatExtensible_Tail	wfx;
	struct Riff64Header				data;
} PACKED;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
