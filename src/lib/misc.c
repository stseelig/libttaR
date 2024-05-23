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

/*@unchecked@*/ /*@unused@*/
const struct LibTTAr_VersionInfo libttaR_info = {
	LIBTTAr_VERSION_NUM,
	LIBTTAr_VERSION_NUM_MAJOR,
	LIBTTAr_VERSION_NUM_MINOR,
	LIBTTAr_VERSION_NUM_REVIS,
	LIBTTAr_VERSION_STR_EXTRA,
	LIBTTAr_VERSION_STR_DATE,
	LIBTTAr_COPYRIGHT_STR,
	LIBTTAr_LICENSE_STR
};

//////////////////////////////////////////////////////////////////////////////

// calculates the size of the private state struct
// returns 0 on error
size_t
libttaR_codecstate_priv_size(uint nchan)
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

// returns whether libttaR was configured to support nchan audio channels
bool
libttaR_test_nchan(uint nchan)
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
