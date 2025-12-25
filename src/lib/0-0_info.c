/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/info.c                                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "../version.h"

#include "./common.h"

/* //////////////////////////////////////////////////////////////////////// */

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

/* //////////////////////////////////////////////////////////////////////// */

/**@struct libttaR_info
 * @brief library version, copyright, and license info
 *
 * @note read the manpage for more info
**/
BUILD_EXPORT
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

/* EOF //////////////////////////////////////////////////////////////////// */
