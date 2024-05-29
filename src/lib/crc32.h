#ifndef TTA_CODEC_CRC32_H
#define TTA_CODEC_CRC32_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/crc32.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "../bits.h"

//////////////////////////////////////////////////////////////////////////////

#define CRC32_INIT	((u32) 0xFFFFFFFFu)

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/ /*@unused@*/
extern const u32 crc32_table[];

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE PURE u32
crc32_cont(register u8 x, register u32 crc)
/*@*/
{
	register const u32 lookup = crc32_table[(crc ^ x) & 0xFFu];
	return (u32) ((crc >> 8u) ^ lookup);
}

ALWAYS_INLINE CONST u32
crc32_end(register u32 crc)
/*@*/
{
	return ~crc;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
