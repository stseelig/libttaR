#ifndef TTA_FORMATS_TTA_H
#define TTA_FORMATS_TTA_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  http://tausoft.org/wiki/True_Audio_Codec_Format                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../../bits.h"

#include "wav.h"	// WAV_FORMAT_PCM

//////////////////////////////////////////////////////////////////////////////

#ifndef LibTTAr_H
#ifndef xENUM_TTASAMPLEBYTES
#define xENUM_TTASAMPLEBYTES
enum TTASampleBytes {
	TTASAMPLEBYTES_1 = 1u,
	TTASAMPLEBYTES_2 = 2u,
	TTASAMPLEBYTES_3 = 3u
};
#define TTA_SAMPLEBYTES_MAX	TTASAMPLEBYTES_3
#define TTA_SAMPLEBITS_MAX	((unsigned int) (8u*TTA_SAMPLEBYTES_MAX))
#endif

#define TTA_FRAME_TIME		((double) 1.04489795918367346939)
#endif

#define TTA1_PREAMBLE		((char[]) {'T','T','A','1'})
#define TTA2_PREAMBLE		((char[]) {'T','T','A','2'})
#define TTA2_SEEKTABLE_SIG ( \
	(u8[3]) {(u8) 0xFFu, (u8) 0xFFu, (u8) 0xFFu} \
)
#define TTA2_FRAMEFOOTER_SIG ( \
	(u8[3]) {(u8) 0xFEu, (u8) 0xFFu, (u8) 0xFFu} \
)

// 13.2 Surround; differs from WAVE channel layout
// http://tausoft.org/wiki/TTA_Channel_Mask
// https://en.wikipedia.org/wiki/10.2_surround_sound
#define TTA_CHAN_FL	((u32) 0x00000001u)
#define TTA_CHAN_FR	((u32) 0x00000002u)
#define TTA_CHAN_FC	((u32) 0x00000004u)
#define TTA_CHAN_LFE	((u32) 0x00000008u)
#define TTA_CHAN_BL	((u32) 0x00000010u)
#define TTA_CHAN_BR	((u32) 0x00000020u)
#define TTA_CHAN_FLC	((u32) 0x00000040u)
#define TTA_CHAN_FRC	((u32) 0x00000080u)
#define TTA_CHAN_BC	((u32) 0x00000100u)
#define TTA_CHAN_SL	((u32) 0x00000200u)
#define TTA_CHAN_SR	((u32) 0x00000400u)
#define TTA_CHAN_TFC	((u32) 0x00000800u)
#define TTA_CHAN_TFL	((u32) 0x00001000u)
#define TTA_CHAN_TFR	((u32) 0x00002000u)
#define TTA_CHAN_LFE2	((u32) 0x00004000u)

//////////////////////////////////////////////////////////////////////////////

// all int's are little-endian

struct TTA1Header {
	char	preamble[4];	// .ascii "TTA1"
	u16	format;		// WAVE_FMT_PCM (0x0001u)
	u16	nchan;		// number of channels
	u16	samplebits;	// bits per sample
	u32	samplerate;	// samples per second
	u32	nsamples;	// number of samples
	u32	crc;		// header CRC
} PACKED;

struct TTA2Header {
	char	preamble[4];	// .ascii "TTA2"
	u16	nchan;		// number of channels
	u16	samplebits;	// bits per sample
	u32	samplerate;	// samples per second
	u32	chanmask;	// TTA_CHAN mask
	u64	nsamples;	// number of samples
	u64	size;		// data block size
	u32	crc;		// header CRC
} PACKED;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
