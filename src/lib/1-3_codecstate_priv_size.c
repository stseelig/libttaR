/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/codecstate_priv_size.c                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stddef.h>

#include "./common.h"
#include "./overflow.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn libttaR_codecstate_priv_size
 * @brief calculates the size of the private state struct
 *
 * @param nchan - number of audio channels
 *
 * @return size of the private state struct suitable for:
 *   aligned_alloc(
 *       LIBTTAr_CODECSTATE_PRIV_ALIGN, libttaR_codecstate_priv_size(nchan)
 *   );
 * @retval 0 - bad 'nchan' (0, or overflow)
 *
 * @note read the manpage for more info
**/
BUILD_EXPORT
CONST
size_t
libttaR_codecstate_priv_size(const unsigned int nchan)
/*@*/
{
	size_t retval, mod;
	size_t temp;
	int overflow_0, overflow_1, overflow_2;

	if ( nchan == 0 ){
		return 0;
	}

	/* base struct size */
	retval = sizeof(struct LibTTAr_CodecState_Priv);

	/* size of the array */
	overflow_0 = mul_usize_overflow(
		&temp, (size_t) nchan, sizeof(struct Codec)
	);

	/* base + array */
	overflow_1 = add_usize_overflow(&retval, retval, temp);

	/* align forward */
	mod    = retval % LIBTTAr_CODECSTATE_PRIV_ALIGN;
	temp   = (mod != 0 ? LIBTTAr_CODECSTATE_PRIV_ALIGN - mod : 0);
	overflow_2 = add_usize_overflow(&retval, retval, temp);

	if ( (overflow_0 != 0) || (overflow_1 != 0) || (overflow_2 != 0) ){
		return 0;
	}
	return retval;
}

/* EOF //////////////////////////////////////////////////////////////////// */
