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

/**@fn seektable_init
 * @brief initializes a seektable
 *
 * @param st[out] the seektable struct
 * @param nframes number of frames in the seektable
**/
void
seektable_init(
	/*@out@*/ struct SeekTable *const restrict st, const size_t nframes
)
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

/**@fn seektable_init
 * @brief add an entry to a seektable
 *
 * @param st[in out] the seektable struct
 * @param value new table entry
 * @param outfile_name[in] destination file name (warnings/errors)
 *
 * @note in encode loop
**/
HOT void
seektable_add(
	struct SeekTable *const restrict st, const size_t value,
	const char *const restrict outfile_name
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
		st->table  = realloc(
			st->table, st->limit * (sizeof *(st->table))
		);
		if UNLIKELY ( st->table == NULL ){
			error_sys(errno, "realloc", NULL);
		}
	}
	assert(st->table != NULL);

	if UNLIKELY ( value > (size_t) UINT32_MAX ){
		warning_tta("%s: frame %zu: seektable entry overflow",
			outfile_name, st->nmemb
		);
	}
	st->table[st->nmemb] = htole32((u32) value);
	++st->nmemb;
	return;
}

/**@fn seektable_init
 * @brief frees any allocated pointers in a seektable
 *
 * @param st[in] the seektable struct
**/
void
seektable_free(const struct SeekTable *const restrict st)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	st->table@*/
{
	free(st->table);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
