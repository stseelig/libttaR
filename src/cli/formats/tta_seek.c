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

#include "../alloc.h"
#include "../debug.h"
#include "../formats.h"

//////////////////////////////////////////////////////////////////////////////

/**@fn seektable_nframes
 * @brief calculates the number of frames in a seektable
 *
 * @param decpcm_size size of the raw PCM
 * @param buflen (framelen * nchan)
 * @param samplebytes bytes per PCM sample
 *
 * @pre (samplebytes != 0) && (decpcm_size != 0) && (buflen != 0)
 *
 * @return the number of frames
**/
CONST size_t
seektable_nframes(
	const size_t decpcm_size, const size_t buflen, const uint samplebytes
)
/*@*/
{
	size_t retval;

	assert((samplebytes != 0) && (decpcm_size != 0) && (buflen != 0));

	retval  = (decpcm_size + buflen) / buflen;
	retval += samplebytes - 1u;
	retval /= samplebytes;
	return retval;
}

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
	st->table = calloc_check(st->limit, sizeof *(st->table));
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
		st->table  = realloc_check(
			st->table, st->limit * (sizeof *(st->table))
		);
	}

	if UNLIKELY ( value > (size_t) UINT32_MAX ){
		warning_tta("%s: frame %zu: seektable entry overflow",
			outfile_name, st->nmemb
		);
	}
	st->table[st->nmemb] = htole32((u32) value);
	st->nmemb           += 1u;
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
