//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta_seek.c                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  http://tausoft.org/wiki/True_Audio_Codec_Format                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include "../../bits.h"

#include "../debug.h"
#include "../formats.h"

//////////////////////////////////////////////////////////////////////////////

void
seektable_init(/*@out@*/ struct SeekTable *const restrict st, size_t nframes)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*st
@*/
/*@allocates	st->table@*/
{
	st->off   = 0;
	st->nmemb = 0;
	st->limit = (nframes != 0 ? nframes : SEEKTABLE_INIT_DEFAULT);
	st->table = calloc(st->limit, sizeof *(st->table));
	if UNLIKELY ( st->table == NULL ){
		error_sys(errno, "calloc", NULL);
	}
	return;
}

// in encode loop
void
seektable_add(
	struct SeekTable *const restrict st, size_t value, size_t framenum,
	const char *outfile_name
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*st
@*/
{
	if ( st->nmemb == st->limit ){
		st->limit += SEEKTABLE_INIT_DEFAULT;
		st->table  = reallocarray(
			st->table, st->limit, (sizeof *(st->table))
		);
		if UNLIKELY ( st->table == NULL ){
			error_sys(errno, "reallocarray", NULL);
		}
	}
	assert(st->table != NULL);

	if UNLIKELY ( value > (size_t) UINT32_MAX ){
		warning_tta("%s: frame %zu: seektable overflow",
			outfile_name, framenum
		);
	}
	st->table[st->nmemb] = htole32((u32) value);
	++st->nmemb;
	return;
}

void
seektable_free(struct SeekTable *const restrict st)
/*@globals	internalState@*/
/*@modifies	internalState,
		*st
@*/
/*@releases	st->table@*/
{
	free(st->table);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
