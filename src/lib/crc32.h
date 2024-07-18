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

#include "../bits.h"

//////////////////////////////////////////////////////////////////////////////

#define CRC32_INIT	((u32) 0xFFFFFFFFu)

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 crc32_table[256u];

//////////////////////////////////////////////////////////////////////////////

/**@fn crc32_cont
 * @brief continue a CRC calculation
 *
 * @param x the byte to add to the CRC
 * @param crc the current CRC
 *
 * @return the updated CRC
**/
ALWAYS_INLINE CONST u32
crc32_cont(const u8 x, const u32 crc)
/*@*/
{
	// having the variable instead of just having a one line return is
	//   important. otherwise it does a xor against memory, which can be
	//   much slower (I think it has something to do with dependency
	//   chains and pipelining). probably a compiler bug
	register const u32 lookup = crc32_table[((u8) crc) ^ x];
	return (u32) ((crc >> 8u) ^ lookup);
}

/**@fn crc32_end
 * @brief end a CRC calculation
 *
 * @param crc the current CRC
 *
 * @return the finished CRC
**/
ALWAYS_INLINE CONST u32
crc32_end(const u32 crc)
/*@*/
{
	return ~crc;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
