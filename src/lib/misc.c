//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/misc.c                                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stddef.h>

#include "../bits.h"
#include "../version.h"

#include "state.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

struct LibTTAr_VersionInfo {
	unsigned int	 version;
	unsigned int	 version_major;
	unsigned int 	 version_minor;
	unsigned int	 version_revis;
	/*@observer@*/
	const char	*version_extra;
	/*@observer@*/
	const char	*version_date;
	/*@observer@*/
	const char	*copyright;
	/*@observer@*/
	const char	*license;
};

/**@struct libttaR_info
 * @brief library version, copyright, and license info
 *
 * @note read the manpage for more info
**/
/*@unchecked@*/ /*@unused@*/
const struct LibTTAr_VersionInfo libttaR_info = {
	LIB_VERSION_NUM,
	LIB_VERSION_NUM_MAJOR,
	LIB_VERSION_NUM_MINOR,
	LIB_VERSION_NUM_REVIS,
	LIB_VERSION_STR_EXTRA,
	LIB_VERSION_STR_DATE,
	LIB_COPYRIGHT_STR,
	LIB_LICENSE_STR
};

//////////////////////////////////////////////////////////////////////////////

/**@fn libttaR_codecstate_priv_size
 * @brief calculates the size of the private state struct
 *
 * @param nchan number of audio channels
 *
 * @return size of the private state struct
 * @retval 0 error
 *
 * @note read the manpage for more info
**/
CONST size_t
libttaR_codecstate_priv_size(const uint nchan)
/*@*/
{
	size_t r = 0;

	if ( nchan == 0 ){
		return r;
	}

	r  = sizeof(struct LibTTAr_CodecState_Priv);
	r += (size_t) (nchan * sizeof(struct Codec));
	return r;
}

/**@fn libttaR_ttabuf_safety_margin_size
 * @brief calculates the size of safety margin for the TTA buffer
 *
 * @param samplebytes number of bytes per PCM sample
 * @param nchan number of audio channels
 *
 * @return size of safety margin
 * @retval 0 error
 *
 * @note read the manpage for more info
**/
CONST size_t
libttaR_ttabuf_safety_margin(
	const enum TTASampleBytes samplebytes, const uint nchan
)
/*@*/
{
	if ( ((uint) samplebytes == 0)
	    ||
	     ((uint) samplebytes > TTA_SAMPLEBYTES_MAX)
	){
		return 0;
	}
	return (size_t) (safety_margin_perchan(samplebytes) * nchan);
}

/**@fn libttaR_test_nchan
 * @brief tests whether libttaR was configured to support 'nchan' audio
 *   channels
 *
 * @param nchan number of audio channels
 *
 * @return true or false
 *
 * @note read the manpage for more info
**/
CONST bool
libttaR_test_nchan(const uint nchan)
/*@*/
{
	bool r = false;

	switch ( nchan ){
	case 0:
		break;
	case 1u:
#ifndef LIBTTAr_DISABLE_MCH
		r = true;
#elif !defined(LIBTTAr_DISABLE_UNROLLED_1CH)
		r = true;
#endif
		break;
	case 2u:
#ifndef LIBTTAr_DISABLE_MCH
		r = true;
#elif !defined(LIBTTAr_DISABLE_UNROLLED_2CH)
		r = true;
#endif
		break;
	default:
#ifndef LIBTTAr_DISABLE_MCH
		r = true;
#endif
		break;
	}

	return r;
}

// EOF ///////////////////////////////////////////////////////////////////////
