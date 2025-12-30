/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/ttabuf_safety_margin.c                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stddef.h>

#include "./common.h"
#include "./tta.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn libttaR_ttabuf_safety_margin_size
 * @brief calculates the size of safety margin for the TTA buffer
 *
 * @param samplebytes - number of bytes per PCM sample
 * @param nchan       - number of audio channels
 *
 * @return size of safety margin
 * @retval 0 - bad value (0 or out of range enum) or overflow
 *
 * @note read the manpage for more info
**/
BUILD_EXPORT
CONST
size_t
libttaR_ttabuf_safety_margin(
	const enum LibTTAr_SampleBytes samplebytes, const unsigned int nchan
)
/*@*/
{
	if ( ((unsigned int) samplebytes == 0)
	    ||
	     ((unsigned int) samplebytes > LIBTTAr_SAMPLEBYTES_MAX)
	){
		return 0;
	}

	return get_safety_margin(samplebytes, nchan);
}

/* EOF //////////////////////////////////////////////////////////////////// */
