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
extern HIDDEN const u32 crc32_table[];

//////////////////////////////////////////////////////////////////////////////

/**@fn crc32_cont
 * @brief continue a CRC calculation
 *
 * @param x the byte to add to the CRC
 * @param crc the current CRC
 *
 * @return the updated CRC
 *
 * @note affected by LIBTTAr_OPT_MEM_XOR
**/
ALWAYS_INLINE CONST u32
crc32_cont(register u8 x, register u32 crc)
/*@*/
{
	// having the variable instead of just having a one line return is
	//   important. otherwise it does a xor against memory, which is much
	//   slower on modern x86. probably a compiler bug
#ifndef LIBTTAr_OPT_MEM_XOR
	register const u32 lookup = crc32_table[((u8) crc) ^ x];
	return (u32) ((crc >> 8u) ^ lookup);
#else
	return (u32) ((crc >> 8u) ^ crc32_table[((u8) crc) ^ x]);
#endif
}

/**@fn crc32_end
 * @brief end a CRC calculation
 *
 * @param crc the current CRC
 *
 * @return the finished CRC
**/
ALWAYS_INLINE CONST u32 crc32_end(register u32 crc) /*@*/ { return ~crc; }

// EOF ///////////////////////////////////////////////////////////////////////
#endif
