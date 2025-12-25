/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/test_nchan.c                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "./common.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn libttaR_test_nchan
 * @brief tests whether libttaR was configured to support 'nchan' audio
 *   channels
 *
 * @param nchan - number of audio channels
 *
 * @retval 0 - not supported
 *
 * @note read the manpage for more info
**/
BUILD_EXPORT
CONST
int
libttaR_test_nchan(const unsigned int nchan)
/*@*/
{
	int retval = 0;
	switch ( nchan ){

#ifndef LIBTTAr_OPT_DISABLE_MCH
	case 0:
		break;
	default:
		retval = (int) nchan;
		break;

#else	/* !defined(LIBTTAr_OPT_DISABLE_MCH) */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
	case 1u:
		retval = (int) nchan;
		break;
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_1CH */

#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
	case 2u:
		retval = (int) nchan;
		break;
#endif	/* LIBTTAr_OPT_DISABLE_UNROLLED_2CH */

	default:
		break;

#endif	/* LIBTTAr_OPT_DISABLE_MCH */
	}
	return retval;
}

/* EOF //////////////////////////////////////////////////////////////////// */
