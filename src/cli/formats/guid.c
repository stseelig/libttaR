//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/guid.c                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <inttypes.h>
#include <stdio.h>

#include "guid.h"

#include "../../splint.h"

//////////////////////////////////////////////////////////////////////////////

// formats buf to:
//	00000000-1111-2222-3333-444444444444
// buf_len should be GUID128_BUFSIZE
// returns buf
char *
guid128_format(
	/*@returned@*/ /*@out@*/ char *const restrict buf, size_t buflen,
	const struct Guid128 *const restrict guid
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
