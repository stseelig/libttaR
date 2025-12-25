/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// formats/metatags_skip.c                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "../ascii-literals.h"
#include "../common.h"
#include "../byteswap.h"
#include "../formats.h"

/* //////////////////////////////////////////////////////////////////////// */

#define APETAG_PREAMBLE	((uint8_t[8u]) { \
	ASCII_A_UP, ASCII_P_UP, ASCII_E_UP, ASCII_T_UP, \
	ASCII_A_UP, ASCII_G_UP, ASCII_E_UP, ASCII_X_UP \
})

#define ID3_PREAMBLE	((uint8_t[3u]) { \
	ASCII_I_UP, ASCII_D_UP, ASCII_3 \
})

/* //////////////////////////////////////////////////////////////////////// */

struct ApeTagHF {
	uint8_t		preamble[8u];	/* .ascii "APETAGEX"               */
	uint32_t	version;	/* 2000 for apetag 2.0             */
	uint32_t	size;		/* length of (items-blob + footer) */
	uint32_t	nmemb;		/* number of items in the tag      */
	uint32_t	apeflags;	/* global flags for tag            */
	uint8_t 	x_pad[8u];	/* padding; all zeroes             */
} PACKED;

struct ID3TagHeader {
	uint8_t		preamble[3u];	/* .ascii "ID3"                    */
	uint16_t	version;
	uint8_t		flags;
	uint8_t		size[4u];	/* length of the tag - header      */
					/* "syncsafe" integer; 7 bit bytes */
} PACKED;

/* //////////////////////////////////////////////////////////////////////// */

#undef file
static enum FileCheck apetag_skip(FILE *RESTRICT file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

#undef file
static enum FileCheck id3tag_skip(FILE *RESTRICT file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
;

CONST
static uint32_t id3syncsafeint(uint8_t, uint8_t, uint8_t, uint8_t) /*@*/;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn metatags_skip
 * @brief advances the file stream past any number of ape/id3 tags
 *
 * @param file - input file
 *
 * @return FILECHECK_MISMATCH on success
**/
BUILD enum FileCheck
metatags_skip(FILE *const RESTRICT file)
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
	}
	while ( retval == FILECHECK_OK );

	return retval;
}

/* ======================================================================== */

/**@fn apetag_skip
 * @brief seek past an apetag
 *
 * @param file - input file
 *
 * @return FILECHECK_OK on success
**/
static enum FileCheck
apetag_skip(FILE *const RESTRICT file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
{
	struct ApeTagHF hdr;
	off_t start;
	union {	size_t	z;
		int	d;
	} result;

	start = ftello(file);

	result.z = fread(&hdr, sizeof hdr, SIZE_C(1), file);
	if ( result.z != SIZE_C(1) ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	/* check for tag */
	result.d = memcmp(
		&hdr.preamble, APETAG_PREAMBLE, sizeof hdr.preamble
	);
	if ( result.d != 0 ){
		/* reset file if no tag */
		result.d = fseeko(file, start, SEEK_SET);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	/* seek to end of tag */
	result.d = fseeko(
		file, (off_t) byteswap_letoh_u32(hdr.size), SEEK_CUR
	);
	if ( result.d != 0 ){
		return FILECHECK_SEEK_ERROR;
	}
	return FILECHECK_OK;
}

/* ------------------------------------------------------------------------ */

/**@fn id3tag_skip
 * @brief seek past an id3tag
 *
 * @param file - input file
 *
 * @return FILECHECK_OK on success
 *
 * @note haven't been able to properly test this yet; just assuming it works
**/
static enum FileCheck
id3tag_skip(FILE *const RESTRICT file)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		file
@*/
{
	struct ID3TagHeader hdr;
	off_t start;
	uint32_t tag_size;
	union {	int	d;
		size_t	z;
	} result;

	start = ftello(file);

	result.z = fread(&hdr, sizeof hdr, SIZE_C(1), file);
	if ( result.z != SIZE_C(1) ){
		if ( feof(file) != 0 ){
			return FILECHECK_MALFORMED;
		}
		return FILECHECK_READ_ERROR;
	}

	/* check for tag */
	result.d = memcmp(&hdr.preamble, ID3_PREAMBLE, sizeof hdr.preamble);
	if ( result.d != 0 ){
		/* reset file if no tag */
		result.d = fseeko(file, start, SEEK_SET);
		if ( result.d != 0 ){
			return FILECHECK_SEEK_ERROR;
		}
		return FILECHECK_MISMATCH;
	}

	/* seek to end of tag */
	tag_size = id3syncsafeint(
		hdr.size[0u], hdr.size[1u], hdr.size[2u], hdr.size[3u]
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
CONST
static uint32_t
id3syncsafeint(
	const uint8_t x0, const uint8_t x1, const uint8_t x2, const uint8_t x3
)
/*@*/
{
	uint32_t retval = 0;

	retval |= (x0 & 0x7Fu);
	retval |= (x1 & 0x7Fu) <<  7u;
	retval |= (x2 & 0x7Fu) << 14u;
	retval |= (x3 & 0x7Fu) << 21u;

	return retval;
}

/* EOF //////////////////////////////////////////////////////////////////// */
