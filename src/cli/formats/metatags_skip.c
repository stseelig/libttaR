//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/metatags_skip.c                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
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
	char	preamble[8];	// .ascii "APETAGEX"
	u32	version;	// 2000 for apetag 2.0
	u32	size;		// length of the items-blob + footer in bytes
	u32	nmemb;		// number of items in the tag
	u32	apeflags;	// global flags for tag
	u8 	pad[8];		// padding; all zeroes
} PACKED;

struct ID3TagHeader {
	char	preamble[3];	// .ascii "ID3"
	u16	version;
	u8	flags;
	u8	size[4];	// length of the tag - header
				// "syncsafe" integer; only lowest 7 bits
} PACKED;

//////////////////////////////////////////////////////////////////////////////

#undef file
static enum FileCheck apetag_skip(FILE *const restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

#undef file
static enum FileCheck id3tag_skip(FILE *const restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

static inline u32 id3syncsafeint(
	register u8, register u8, register u8, register u8
)
/*@*/
;

//////////////////////////////////////////////////////////////////////////////

// advances the file stream past any number of ape/id3 tags
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

// seeks past an apetag
static enum FileCheck
apetag_skip(FILE *const restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
{
	struct ApeTagHF h;
	off_t start;
	union {
		size_t	z;
		int	d;
	} t;

	start = ftello(file);

	t.z = fread(&h, sizeof h, (size_t) 1, file);
	if ( t.z != (size_t) 1 ){
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

// seeks past an id3tag
// TODO haven't been able to properly test this yet; just assuming it works
static enum FileCheck
id3tag_skip(FILE *const restrict file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
{
	struct ID3TagHeader h;
	off_t start;
	union {
		size_t	z;
		int	d;
	} t;

	start = ftello(file);

	t.z = fread(&h, sizeof h, (size_t) 1, file);
	if ( t.z != (size_t) 1 ){
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
	t.z = id3syncsafeint(h.size[0], h.size[1], h.size[2], h.size[3]);
	t.d = fseeko(file, (off_t) t.z, SEEK_CUR);
	if ( t.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	return FILECHECK_OK;
}

static inline u32
id3syncsafeint(register u8 x0, register u8 x1, register u8 x2, register u8 x3)
/*@*/
{
	register u32 r = 0;
	r |= (x0 & 0x7Fu);
	r |= (x1 & 0x7Fu) <<  7u;
	r |= (x2 & 0x7Fu) << 14u;
	r |= (x3 & 0x7Fu) << 21u;
	return r;
}

// EOF ///////////////////////////////////////////////////////////////////////
