//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/metatags_skip.c                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>
#include <string.h>	// memcmp

#include "../../bits.h"

#include "../formats.h"

//////////////////////////////////////////////////////////////////////////////

#define APETAG_PREAMBLE	((char[]) {'A','P','E','T','A','G','E','X'})
#define ID3_PREAMBLE	((char[]) {'I','D','3'})

//////////////////////////////////////////////////////////////////////////////

struct ApeTagHF {
	char	preamble[8u];	// .ascii "APETAGEX"
	u32	version;	// 2000 for apetag 2.0
	u32	size;		// length of the items-blob + footer in bytes
	u32	nmemb;		// number of items in the tag
	u32	apeflags;	// global flags for tag
	u8 	pad[8u];	// padding; all zeroes
} PACKED;

struct ID3TagHeader {
	char	preamble[3u];	// .ascii "ID3"
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
	enum FileCheck r, fc0, fc1;

	do {	fc0 = apetag_skip(file);
		fc1 = id3tag_skip(file);

		if ( (fc0 == FILECHECK_OK) || (fc1 == FILECHECK_OK) ){
			r = FILECHECK_OK;
		}
		else if ( fc0 != FILECHECK_MISMATCH ){
			r = fc0;
		}
		else if ( fc1 != FILECHECK_MISMATCH ){
			r = fc1;
		}
		else {	r = fc0; }
	} while ( r == FILECHECK_OK );

	return r;
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
	} t;

	start = ftello(file);

	t.z = fread(&h, sizeof h, (size_t) 1u, file);
	if ( t.z != (size_t) 1u ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	// check for tag
	t.d = memcmp(&h.preamble, APETAG_PREAMBLE, sizeof h.preamble);
	if ( t.d != 0 ){
		// reset file if no tag
		t.d = fseeko(file, start, SEEK_SET);
		if ( t.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	// seek to end of tag
	t.d = fseeko(file, (off_t) h.size, SEEK_CUR);
	if ( t.d != 0 ){
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
	union {	size_t	z;
		int	d;
	} t;

	start = ftello(file);

	t.z = fread(&h, sizeof h, (size_t) 1u, file);
	if ( t.z != (size_t) 1u ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	// check for tag
	t.d = memcmp(&h.preamble, ID3_PREAMBLE, sizeof h.preamble);
	if ( t.d != 0 ){
		// reset file if no tag
		t.d = fseeko(file, start, SEEK_SET);
		if ( t.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	// seek to end of tag
	t.z = id3syncsafeint(h.size[0], h.size[1u], h.size[2u], h.size[3u]);
	t.d = fseeko(file, (off_t) t.z, SEEK_CUR);
	if ( t.d != 0 ){
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
 * @note IMO, it is an over-engineered solution to a non-existent problem.
**/
static CONST u32
id3syncsafeint(const u8 x0, const u8 x1, const u8 x2, const u8 x3)
/*@*/
{
	u32 r = 0;
	r |= (x0 & 0x7Fu);
	r |= (x1 & 0x7Fu) <<  7u;
	r |= (x2 & 0x7Fu) << 14u;
	r |= (x3 & 0x7Fu) << 21u;
	return r;
}

// EOF ///////////////////////////////////////////////////////////////////////
