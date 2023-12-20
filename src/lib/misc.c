//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/misc.c                                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>

#include "../bits.h"
#include "../version.h"

#include "state.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/ /*@unused@*/
const uint libttaR_num_version= LIBTTAr_NUM_VERSION;
/*@unchecked@*/ /*@unused@*/
const uint libttaR_num_version_major = LIBTTAr_NUM_VERSION_MAJOR;
/*@unchecked@*/ /*@unused@*/
const uint libttaR_num_version_minor = LIBTTAr_NUM_VERSION_MINOR;
/*@unchecked@*/ /*@unused@*/
const uint libttaR_num_version_revis = LIBTTAr_NUM_VERSION_REVIS;

/*@observer@*/ /*@unchecked@*/ /*@unused@*/
const char *libttaR_str_version = LIBTTAr_STR_VERSION;

/*@observer@*/ /*@unchecked@*/ /*@unused@*/
const char *libttaR_str_copyright = LIBTTAr_STR_COPYRIGHT;

/*@observer@*/ /*@unchecked@*/ /*@unused@*/
const char *libttaR_str_license = LIBTTAr_STR_LICENSE;

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
	r += nchan * sizeof(struct Codec);
	return r;
}

// returns a TTA buffer size that "should" be safe
// returns 0 on error
size_t
libttaR_ttabuf_size(
	size_t nsamples, uint nchan, enum TTASampleBytes samplebytes
)
/*@*/
{	size_t r = 0;

	if ( (nsamples == 0) || (nchan == 0)
	    ||
	     (samplebytes == 0) || (samplebytes > TTA_SAMPLEBYTES_MAX)
	){
		return r;
	}

	r  = nsamples + TTABUF_SAFETY_MARGIN;
	r *= nchan * samplebytes;
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
