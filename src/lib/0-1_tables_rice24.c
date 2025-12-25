/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tables_rice24.c                                                    //
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

/* //////////////////////////////////////////////////////////////////////// */

/**@var binexp32p4_table
 * @brief binary exponetiation 32-bit + 4-lshift (2**('k' + 4u)) lookup table
 *   a value with only the ('k' + 4u)th bit set, 0, or 0xFFFFFFFFu
 * @note special cases (for rice24_update):
 *        0u => 0x00000000u: floors rice.k[] to  0u
 *       25u => 0xFFFFFFFFu:   caps rice.k[] to 24u
**/
/*@unchecked@*/
BUILD_HIDDEN
const uint32_t binexp32p4_table[26u] = {
#define T_C(x_x)	UINT32_C(x_x)
T_C(0x00000000), T_C(0x00000020), T_C(0x00000040), T_C(0x00000080),
T_C(0x00000100), T_C(0x00000200), T_C(0x00000400), T_C(0x00000800),
T_C(0x00001000), T_C(0x00002000), T_C(0x00004000), T_C(0x00008000),
T_C(0x00010000), T_C(0x00020000), T_C(0x00040000), T_C(0x00080000),
T_C(0x00100000), T_C(0x00200000), T_C(0x00400000), T_C(0x00800000),
T_C(0x01000000), T_C(0x02000000), T_C(0x04000000), T_C(0x08000000),
T_C(0x10000000), T_C(0xFFFFFFFF)
#undef T_C
};

#ifdef LIBTTAr_OPT_PREFER_LOOKUP_TABLES
/**@var lsmask32_table
 * @brief least significant mask 32-bit lookup table
**/
/*@unchecked@*/
BUILD_HIDDEN
const uint32_t lsmask32_table[32u] = {
#define T_C(x_x)	UINT32_C(x_x)
T_C(0x00000000), T_C(0x00000001), T_C(0x00000003), T_C(0x00000007),
T_C(0x0000000F), T_C(0x0000001F), T_C(0x0000003F), T_C(0x0000007F),
T_C(0x000000FF), T_C(0x000001FF), T_C(0x000003FF), T_C(0x000007FF),
T_C(0x00000FFF), T_C(0x00001FFF), T_C(0x00003FFF), T_C(0x00007FFF),
T_C(0x0000FFFF), T_C(0x0001FFFF), T_C(0x0003FFFF), T_C(0x0007FFFF),
T_C(0x000FFFFF), T_C(0x001FFFFF), T_C(0x003FFFFF), T_C(0x007FFFFF),
T_C(0x00FFFFFF), T_C(0x01FFFFFF), T_C(0x03FFFFFF), T_C(0x07FFFFFF),
T_C(0x0FFFFFFF), T_C(0x1FFFFFFF), T_C(0x3FFFFFFF), T_C(0x7FFFFFFF)
#undef T_C
};
#endif /* LIBTTAr_OPT_PREFER_LOOKUP_TABLES */

/* EOF //////////////////////////////////////////////////////////////////// */
