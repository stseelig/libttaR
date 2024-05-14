#ifndef TTA_TTA2_H
#define TTA_TTA2_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// tta2.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t
#include <stdio.h>	// FILE

#include "../bits.h"
#include "../libttaR.h"

#include "bufs.h"
#include "formats.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////////

#undef buf
INLINE uint dec_frame_zeropad(
	register i32 *const restrict buf, register size_t, register uint,
	register uint
)
/*@modifies	*buf@*/
;

//==========================================================================//

// tta2dec_st

#undef seektable
#undef dstat_out
#undef outfile
#undef infile
extern void ttadec_loop_st(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct DecStats *const restrict dstat_out,
	const struct FileStats *const restrict,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*seektable,
		*dstat_out,
		outfile,
		infile
@*/
;

//////////////////////////////////////////////////////////////////////////////

// returns nmemb of buf zero-padded
INLINE uint
dec_frame_zeropad(
	register i32 *const restrict buf, register size_t ni32,
	register uint diff, register uint nchan
)
/*@modifies	*buf@*/
{
	register const size_t r = (size_t) (nchan - diff);
	memset(&buf[ni32], 0x00, r * (sizeof *buf));
	return (uint) r;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
