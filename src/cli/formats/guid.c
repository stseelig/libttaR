//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/guid.c                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef S_SPLINT_S
#include "../../splint.h"
#endif

/* ------------------------------------------------------------------------ */

#include <inttypes.h>
#include <stdio.h>

#include "guid.h"

//////////////////////////////////////////////////////////////////////////////

/**@fn guid128_format
 * @brief prints a GUID to a buffer: 00000000-1111-2222-3333-444444444444
 *
 * @param buf[out] the destination buffer
 * @param buflen the size of the destination buffer (GUID128_BUFLEN)
 * @param guid[in] the GUID to format
 *
 * @return 'buf'
 *
 * @note only used for an error message
**/
COLD char *
guid128_format(
	/*@returned@*/ /*@out@*/ char *const restrict buf,
	const size_t buflen, const struct Guid128 *const restrict guid
)
/*@modifies	*buf@*/
{
	// first 3 are little-endian, last 2 are big-endian
	(void) snprintf(buf, buflen,
		"%02"PRIX8"%02"PRIX8"%02"PRIX8"%02"PRIX8
		"-"
		"%02"PRIX8"%02"PRIX8
		"-"
		"%02"PRIX8"%02"PRIX8
		"-"
		"%02"PRIX8"%02"PRIX8
		"-"
		"%02"PRIX8"%02"PRIX8"%02"PRIX8"%02"PRIX8"%02"PRIX8"%02"PRIX8,
		guid->d0[3u], guid->d0[2u], guid->d0[1u], guid->d0[0u],
		guid->d1[1u], guid->d1[0u],
		guid->d2[1u], guid->d2[0u],
		guid->d3[0u], guid->d3[1u],
		guid->d4[0u], guid->d4[1u], guid->d4[2u], guid->d4[3u],
		guid->d4[4u], guid->d4[5u]
	);
	return buf;
}

// EOF ///////////////////////////////////////////////////////////////////////
