#ifndef H_TTA_FORMATS_TTA_H
#define H_TTA_FORMATS_TTA_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  http://tausoft.org/wiki/True_Audio_Codec_Format                         //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>

#include "../ascii-literals.h"
#include "../common.h"

/* //////////////////////////////////////////////////////////////////////// */

#define TTA1_PREAMBLE	((uint8_t[4u]) { \
	ASCII_T_UP, ASCII_T_UP, ASCII_A_UP, ASCII_1 \
})

/* ------------------------------------------------------------------------ */

#define TTA2_PREAMBLE	((uint8_t[4u]) { \
	ASCII_T_UP, ASCII_T_UP, ASCII_A_UP, ASCII_2 \
})

#define TTA2_SEEKTABLE_SIG	((uint8_t[3u]) { \
	UINT8_C(0xFF), UINT8_C(0xFF), UINT8_C(0xFF) \
})

#define TTA2_FRAMEFOOTER_SIG	((uint8_t[3u]) { \
	UINT8_C(0xFE), UINT8_C(0xFF), UINT8_C(0xFF) \
})

/* 13.2 Surround; differs from WAVE channel layout
	- http://tausoft.org/wiki/TTA_Channel_Mask
	- https://en.wikipedia.org/wiki/10.2_surround_sound
*/
#define TTA2_CHAN_FL	UINT32_C(0x00000001)
#define TTA2_CHAN_FR	UINT32_C(0x00000002)
#define TTA2_CHAN_FC	UINT32_C(0x00000004)
#define TTA2_CHAN_LFE	UINT32_C(0x00000008)
#define TTA2_CHAN_BL	UINT32_C(0x00000010)
#define TTA2_CHAN_BR	UINT32_C(0x00000020)
#define TTA2_CHAN_FLC	UINT32_C(0x00000040)
#define TTA2_CHAN_FRC	UINT32_C(0x00000080)
#define TTA2_CHAN_BC	UINT32_C(0x00000100)
#define TTA2_CHAN_SL	UINT32_C(0x00000200)
#define TTA2_CHAN_SR	UINT32_C(0x00000400)
#define TTA2_CHAN_TFC	UINT32_C(0x00000800)
#define TTA2_CHAN_TFL	UINT32_C(0x00001000)
#define TTA2_CHAN_TFR	UINT32_C(0x00002000)
#define TTA2_CHAN_LFE2	UINT32_C(0x00004000)

/* //////////////////////////////////////////////////////////////////////// */

/* all int's are little-endian */

struct TTA1Header {
	uint8_t		preamble[4u];	/* .ascii "TTA1"          */
	uint16_t	format;		/* WAVE_FMT_PCM (0x0001u) */
	uint16_t	nchan;		/* number of channels     */
	uint16_t	samplebits;	/* bits per sample        */
	uint32_t	samplerate;	/* samples per second     */
	uint32_t	nsamples;	/* number of samples      */
	uint32_t	crc;		/* header CRC             */
} PACKED;

struct TTA2Header {
	uint8_t		preamble[4u];	/* .ascii "TTA2"          */
	uint16_t	nchan;		/* number of channels     */
	uint16_t	samplebits;	/* bits per sample        */
	uint32_t	samplerate;	/* samples per second     */
	uint32_t	chanmask;	/* TTA_CHAN mask          */
	uint64_t	nsamples;	/* number of samples      */
	uint64_t	size;		/* data block size        */
	uint32_t	crc;		/* header CRC             */
} PACKED;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_FORMATS_TTA_H */
