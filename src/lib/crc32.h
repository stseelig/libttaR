#ifndef H_TTA_CODEC_CRC32_H
#define H_TTA_CODEC_CRC32_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/crc32.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>

#include "./common.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

#define CRC32_INIT	UINT32_MAX
#define CRC32_FINI(x_x)	(~(x_x))

/* //////////////////////////////////////////////////////////////////////// */

/*@-redef@*/
/*@unchecked@*/
BUILD_HIDDEN
BUILD_EXTERN const uint32_t crc32_table[256u];
/*@=redef@*/

/* //////////////////////////////////////////////////////////////////////// */

/* having the variable instead of just having a one line return is important.
     otherwise it does (on x86) a xor against memory, which can be much slower
     (I think it has to do with dependency chains and pipelining). probably a
     compiler bug.
*/
#define CRC32_CONT_BODY(x_type) { \
	register const uint32_t lookup = crc32_table[((uint8_t) crc) ^ x]; \
	\
	return (x_type) ((crc >> 8u) ^ lookup); \
}

/**@fn crc32_cont
 * @brief continue a CRC calculation
 *
 * @param x the byte to add to the CRC
 * @param crc the current CRC
 *
 * @return the updated CRC
**/
CONST
ALWAYS_INLINE uint32_t
crc32_cont(const uint8_t x, const uint32_t crc)
/*@*/
{
	CRC32_CONT_BODY(uint32_t);
}

/**@fn crc32_cont_enc
 * @brief continue a CRC calculation; encode version
 *
 * @see crc32_cont()
**/
CONST
ALWAYS_INLINE crc32_enc
crc32_cont_enc(const uint8_t x, const crc32_enc crc)
/*@*/
{
	CRC32_CONT_BODY(crc32_enc);
}

/**@fn crc32_cont_dec
 * @brief continue a CRC calculation; decode version
 *
 * @see crc32_cont()
**/
CONST
ALWAYS_INLINE crc32_dec
crc32_cont_dec(const uint8_t x, const crc32_dec crc)
/*@*/
{
	CRC32_CONT_BODY(crc32_dec);
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_CRC32_H */
