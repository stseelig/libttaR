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

#include <limits.h>	// UINT32_MAX

#include "../bits.h"

#include "common.h"

//////////////////////////////////////////////////////////////////////////////

#define CRC32_INIT	UINT32_MAX
#define CRC32_FINI(x)	(~(x))

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/ /*@unused@*/
extern HIDDEN const u32 crc32_table[256u];

//////////////////////////////////////////////////////////////////////////////

// having the variable instead of just having a one line return is important.
//   otherwise it does (on x86) a xor against memory, which can be much slower
//   (I think it has to do with dependency chains and pipelining). probably a
//   compiler bug.
#define CRC32_CONT_BODY(Xtype) { \
	register const u32 lookup = crc32_table[((u8) crc) ^ x]; \
	return (Xtype) ((crc >> 8u) ^ lookup); \
}

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
	CRC32_CONT_BODY(u32);
}

/**@fn crc32_cont_enc
 * @brief continue a CRC calculation; encode version
 *
 * @see crc32_cont()
**/
ALWAYS_INLINE CONST crc32_enc
crc32_cont_enc(const u8 x, const crc32_enc crc)
/*@*/
{
	CRC32_CONT_BODY(crc32_enc);
}

/**@fn crc32_cont_dec
 * @brief continue a CRC calculation; decode version
 *
 * @see crc32_cont()
**/
ALWAYS_INLINE CONST crc32_dec
crc32_cont_dec(const u8 x, const crc32_dec crc)
/*@*/
{
	CRC32_CONT_BODY(crc32_dec);
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
