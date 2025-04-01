//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/metatags_skip.c                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>	// memcmp

#include "../../bits.h"

#include "../formats.h"

//////////////////////////////////////////////////////////////////////////////

#define APETAG_PREAMBLE	((u8[]) \
	{(u8)'A',(u8)'P',(u8)'E',(u8)'T',(u8)'A',(u8)'G',(u8)'E',(u8)'X'} \
)
#define ID3_PREAMBLE	((u8[]) {(u8)'I',(u8)'D',(u8)'3'})

//////////////////////////////////////////////////////////////////////////////

struct ApeTagHF {
	u8	preamble[8u];	// .ascii "APETAGEX"
	u32	version;	// 2000 for apetag 2.0
	u32	size;		// length of the items-blob + footer in bytes
	u32	nmemb;		// number of items in the tag
	u32	apeflags;	// global flags for tag
	u8 	pad[8u];	// padding; all zeroes
} PACKED;

struct ID3TagHeader {
	u8	preamble[3u];	// .ascii "ID3"
	u16	version;
	u8	flags;
	u8	size[4u];	// length of the tag - header
				// "syncsafe" integer; only lowest 7 bits
} PACKED;

//////////////////////////////////////////////////////////////////////////////

#undef file
static enum FileCheck apetag_skip(FILE *restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

#undef file
static enum FileCheck id3tag_skip(FILE *restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

static CONST u32 id3syncsafeint(u8, u8, u8, u8) /*@*/;

//////////////////////////////////////////////////////////////////////////////

/**@fn metatags_skip
 * @brief advances the file stream past any number of ape/id3 tags
 *
 * @param file[in] the input file
 *
 * @return FILECHECK_MISMATCH on success
**/
enum FileCheck
metatags_skip(FILE *const restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
{
	enum FileCheck retval, fc0, fc1;

	do {	fc0 = apetag_skip(file);
		fc1 = id3tag_skip(file);

		if ( (fc0 == FILECHECK_OK) || (fc1 == FILECHECK_OK) ){
			retval = FILECHECK_OK;
		}
		else if ( fc0 != FILECHECK_MISMATCH ){
			retval = fc0;
		}
		else if ( fc1 != FILECHECK_MISMATCH ){
			retval = fc1;
		}
		else {	retval = fc0; }
	} while ( retval == FILECHECK_OK );

	return retval;
}

//==========================================================================//

/**@fn apetag_skip
 * @brief seek past an apetag
 *
 * @param file[in] the input file
 *
 * @return FILECHECK_OK on success
**/
static enum FileCheck
apetag_skip(FILE *const restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
{
	struct ApeTagHF h;
	off_t start;
	union {	size_t	z;
		int	d;
	} result;

	start = ftello(file);

	result.z = fread(&h, sizeof h, (size_t) 1u, file);
	if ( result.z != (size_t) 1u ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	// check for tag
	result.d = memcmp(&h.preamble, APETAG_PREAMBLE, sizeof h.preamble);
	if ( result.d != 0 ){
		// reset file if no tag
		result.d = fseeko(file, start, SEEK_SET);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	// seek to end of tag
	result.d = fseeko(file, (off_t) h.size, SEEK_CUR);
	if ( result.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	return FILECHECK_OK;
}

//--------------------------------------------------------------------------//

/**@fn id3tag_skip
 * @brief seek past an id3tag
 *
 * @param file[in] the input file
 *
 * @return FILECHECK_OK on success
 *
 * @note haven't been able to properly test this yet; just assuming it works
**/
static enum FileCheck
id3tag_skip(FILE *const restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
{
	struct ID3TagHeader h;
	off_t start;
	u32 tag_size;
	union {	int	d;
		size_t	z;
	} result;

	start = ftello(file);

	result.z = fread(&h, sizeof h, (size_t) 1u, file);
	if ( result.z != (size_t) 1u ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	// check for tag
	result.d = memcmp(&h.preamble, ID3_PREAMBLE, sizeof h.preamble);
	if ( result.d != 0 ){
		// reset file if no tag
		result.d = fseeko(file, start, SEEK_SET);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	// seek to end of tag
	tag_size = id3syncsafeint(
		h.size[0], h.size[1u], h.size[2u], h.size[3u]
	);
	result.d = fseeko(file, (off_t) tag_size, SEEK_CUR);
	if ( result.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	return FILECHECK_OK;
}

/**@fn id3syncsafeint
 * @brief converts an id3 sync-safe int into something usable
 *
 * @param x0 first byte
 * @param x1 second byte
 * @param x2 third byte
 * @param x3 fourth byte
 *
 * @return the u32
 *
 * @note IMO, it is an over-engineered solution to a non-existent problem.
**/
static CONST u32
id3syncsafeint(const u8 x0, const u8 x1, const u8 x2, const u8 x3)
/*@*/
{
	u32 retval = 0;
	retval |= (x0 & 0x7Fu);
	retval |= (x1 & 0x7Fu) <<  7u;
	retval |= (x2 & 0x7Fu) << 14u;
	retval |= (x3 & 0x7Fu) << 21u;
	return retval;
}

// EOF ///////////////////////////////////////////////////////////////////////
