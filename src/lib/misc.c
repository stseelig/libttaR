//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/misc.c                                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stddef.h>

#include "../bits.h"
#include "../version.h"

#include "common.h"
#include "crc32.h"
#include "tta.h"

//////////////////////////////////////////////////////////////////////////////

struct LibTTAr_VersionInfo {
	uint		 version;
	uint		 version_major;
	uint	 	 version_minor;
	uint		 version_revis;
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

/**@fn libttaR_test_nchan
 * @brief tests whether libttaR was configured to support 'nchan' audio
 *   channels
 *
 * @param nchan number of audio channels
 *
 * @retval 0 - not supported
 *
 * @note read the manpage for more info
 * @note affected by:
 *     LIBTTAr_OPT_DISABLE_UNROLLED_1CH,
 *     LIBTTAr_OPT_DISABLE_UNROLLED_2CH,
 *     LIBTTAr_OPT_DISABLE_MCH
**/
/*@unused@*/
CONST int
libttaR_test_nchan(const uint nchan)
/*@*/
{
	assert(nchan != 0);

	int retval = 0;
	switch ( nchan ){
#ifndef LIBTTAr_OPT_DISABLE_MCH
	case 0:
		break;
	default:
		retval = (int) nchan;
		break;
#else
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_1CH
	case 1u:
		retval = (int) nchan;
		break;
#endif
#ifndef LIBTTAr_OPT_DISABLE_UNROLLED_2CH
	case 2u:
		retval = (int) nchan;
		break;
#endif
	default:
		break;
#endif // LIBTTAr_OPT_DISABLE_MCH
	}
	return retval;
}

/**@fn libttaR_nsamples_perframe_tta1
 * @brief calculates the number of audio samples per TTA1 frame
 *
 * @param samplerate the audio sampling frequency in samples-per-second
 *
 * @return the number of samples in a TTA1 frame for a given 'samplerate'
 *
 * @note read the manpage for more info
**/
/*@unused@*/
CONST size_t
libttaR_nsamples_perframe_tta1(const size_t samplerate)
/*@*/
{
	assert(samplerate != 0);

	return (size_t) ((256u * samplerate) / 245u);
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
/*@unused@*/
CONST size_t
libttaR_ttabuf_safety_margin(
	const enum LibTTAr_SampleBytes samplebytes, const uint nchan
)
/*@*/
{
	SAMPLEBYTES_RANGE_ASSERT(samplebytes);
	assert(nchan != 0);

	if ( ((uint) samplebytes == 0)
	    ||
	     ((uint) samplebytes > LIBTTAr_SAMPLEBYTES_MAX)
	){
		return 0;
	}
	return get_safety_margin(samplebytes, nchan);
}

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
/*@unused@*/
CONST size_t
libttaR_codecstate_priv_size(const uint nchan)
/*@*/
{
	assert(nchan != 0);

	if ( nchan == 0 ){
		return 0;
	}
	return (sizeof(struct LibTTAr_CodecState_Priv)
	     + ((size_t) (nchan * sizeof(struct Codec)))
	);
}

/**@fn libttaR_crc32
 * @brief calculate a TTA Cyclic Redundancy Code
 *
 * @param buf[in] the input buffer
 * @param size size of the buffer
 *
 * @return the CRC
 *
 * @note read the manpage for more info
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
**/
/*@unused@*/
PURE u32
libttaR_crc32(const u8 *const restrict buf, const size_t size)
/*@*/
{
	u32 crc = CRC32_INIT;
	size_t i;

	assert(size != 0);

	for ( i = 0; i < size; ++i ){
		crc = crc32_cont(buf[i], crc);
	}
	return CRC32_FINI(crc);
}

// EOF ///////////////////////////////////////////////////////////////////////
