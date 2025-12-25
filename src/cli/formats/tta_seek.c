/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/tta_seek.c                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  http://tausoft.org/wiki/True_Audio_Codec_Format                         //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../alloc.h"
#include "../byteswap.h"
#include "../common.h"
#include "../debug.h"
#include "../formats.h"

/* //////////////////////////////////////////////////////////////////////// */

/**@fn seektable_nframes
 * @brief calculates the number of frames in a seektable
 *
 * @param decpcm_size - size of the raw PCM
 * @param buflen      - (framelen * nchan)
 * @param samplebytes - bytes per PCM sample
 *
 * @return the number of frames
**/
CONST
BUILD size_t
seektable_nframes(
	const size_t decpcm_size, const size_t buflen,
	const unsigned int samplebytes
)
/*@*/
{
	size_t retval;

	assert((samplebytes != 0) && (decpcm_size != 0) && (buflen != 0));

	retval  = (decpcm_size / samplebytes);
	retval += (uint8_t) (decpcm_size % samplebytes != 0);
	retval  = (retval / buflen) + ((uint8_t) (retval % buflen != 0));

	return retval;
}

/**@fn seektable_init
 * @brief initializes a seektable
 *
 * @param st      - seektable struct
 * @param nframes - number of frames in the seektable
**/
BUILD void
seektable_init(
	/*@out@*/ struct SeekTable *const RESTRICT st, const size_t nframes
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
 * @param st           - seektable struct
 * @param value        - new table entry
 * @param outfile_name - destination file name (warnings/errors)
 *
 * @note in encode loop
**/
HOT
BUILD void
seektable_add(
	struct SeekTable *const RESTRICT st, const size_t value,
	const char *const RESTRICT outfile_name
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
	st->table[st->nmemb] = byteswap_htole_u32((uint32_t) value);
	st->nmemb           += 1u;

	return;
}

/**@fn seektable_init
 * @brief frees any allocated pointers in a seektable
 *
 * @param st - seektable struct
**/
BUILD void
seektable_free(const struct SeekTable *const RESTRICT st)
/*@globals	internalState@*/
/*@modifies	internalState@*/
/*@releases	st->table@*/
{
	free(st->table);

	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
