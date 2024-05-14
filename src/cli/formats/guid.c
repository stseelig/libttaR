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
// buf should have size of GUID128_BUFSIZE
// returns buf
char *
guid128_format(
	/*@returned@*/ /*@out@*/ char *const restrict buf,
	const struct Guid128 *const restrict guid
)
/*@modifies	*buf@*/
{
	uint i, j;

	// first 3 are little-endian, last 2 are big-endian
	j = 0;
	for ( i = (uint) (sizeof guid->d0); i-- > 0; j += 2u ){
		(void) snprintf(
			&buf[j], (size_t) 3u, "%02"PRIX8"", guid->d0[i]
		);
	}
	buf[j++] = '-';
	for ( i = (uint) (sizeof guid->d1); i-- > 0; j += 2u ){
		(void) snprintf(
			&buf[j], (size_t) 3u, "%02"PRIX8"", guid->d1[i]
		);
	}
	buf[j++] = '-';
	for ( i = (uint) (sizeof guid->d2); i-- > 0; j += 2u ){
		(void) snprintf(
			&buf[j], (size_t) 3u, "%02"PRIX8"", guid->d2[i]
		);
	}
	buf[j++] = '-';
	for ( i = 0; i < (uint) (sizeof guid->d3); ++i, j += 2u ){
		(void) snprintf(
			&buf[j], (size_t) 3u, "%02"PRIX8"", guid->d3[i]
		);
	}
	buf[j++] = '-';
	for ( i = 0; i < (uint) (sizeof guid->d4); ++i, j += 2u){
		(void) snprintf(
			&buf[j], (size_t) 3u, "%02"PRIX8"", guid->d4[i]
		);
	}

	return buf;
}

// EOF ///////////////////////////////////////////////////////////////////////
