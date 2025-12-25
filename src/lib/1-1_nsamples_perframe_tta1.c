/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/nsamples_perframe_tta1.c                                           //
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
#include "./overflow.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn libttaR_nsamples_perframe_tta1
 * @brief calculates the number of audio samples per TTA1 frame
 *
 * @param samplerate - audio sampling frequency in samples-per-second
 *
 * @return the number of samples in a TTA1 frame for a given 'samplerate'
 * @retval 0 - bad 'samplerate' (0 or too large (overflow))
 *
 * @note read the manpage for more info
**/
BUILD_EXPORT
CONST
size_t
libttaR_nsamples_perframe_tta1(const size_t samplerate)
/*@*/
{
	size_t temp;
	int overflow;

	overflow = mul_usize_overflow(&temp, samplerate, SIZE_C(256));
	if ( overflow != 0 ){
		return 0;
	}
	return temp / SIZE_C(245);
}

/* EOF //////////////////////////////////////////////////////////////////// */
