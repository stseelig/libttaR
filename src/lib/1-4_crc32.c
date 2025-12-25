/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/crc32.c                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stddef.h>
#include <stdint.h>

#include "./common.h"
#include "./crc32.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn libttaR_crc32
 * @brief calculate a TTA Cyclic Redundancy Code
 *
 * @param buf  - input buffer
 * @param size - size of the buffer
 *
 * @return CRC of the buffer
 *
 * @note width   : 32-bit
 *       endian  : little
 *       poly    : 0xEDB88320u
 *       xor-in  : 0xFFFFFFFFu
 *       xor-out : 0xFFFFFFFFu
 * @note This function is obviously not as fast as Intel's slicing method. It
 *   is meant for TTA's header and seektable. Given that those are rather
 *   small and calculating their CRCs is an insignificant part of a program's
 *   runtime, size is more important. Frame CRC calculation is inlined into
 *   the rice coder.
 * @note read the manpage for more info
**/
BUILD_EXPORT
PURE
uint32_t
libttaR_crc32(const void *const RESTRICT buf, const size_t size)
/*@*/
{
	const uint8_t *const RESTRICT buf_8 = buf;
	/* * */
	uint32_t crc = CRC32_INIT;
	size_t i;

	for ( i = 0; i < size; ++i ){
		crc = crc32_cont(buf_8[i], crc);
	}
	return CRC32_FINI(crc);
}

/* EOF //////////////////////////////////////////////////////////////////// */
