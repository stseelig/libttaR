/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// open.c                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libttaR.h"

#include "./alloc.h"
#include "./common.h"
#include "./debug.h"
#include "./main.h"
#include "./open.h"
#include "./opts.h"
#include "./system.h"

/* //////////////////////////////////////////////////////////////////////// */

static int try_fdlimit(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

/* ------------------------------------------------------------------------ */

#undef fstat
#undef file
static enum FileCheck filecheck_codecfmt(
	struct FileStats *RESTRICT fstat, FILE *RESTRICT file,
	const char *RESTRICT, enum ProgramMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		fstat,
		file
@*/
;

/* ------------------------------------------------------------------------ */

/*@only@*/
static char *
outfile_name_fmt(
	/*@null@*/ const char *, const char *, /*@null@*/ const char *
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

PURE
/*@temp@*/ /*@null@*/
static char *findrchar(const char *RESTRICT, char, size_t) /*@*/;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn fopen_check
 * @brief open a file with error checking
 *
 * @param pathname - name of the file
 * @param mode     - name of the mode
 * @param fatality - whether an error is fatal or non-fatal
 *
 * @return pointer to the opened FILE on success, else NULL
**/
/*@dependent@*/ /*@null@*/
BUILD FILE *
fopen_check(
	const char *const RESTRICT pathname, const char *const RESTRICT mode,
	const enum Fatality fatality
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	FILE *retval;

try_again:
	retval = fopen(pathname, mode);
	if UNLIKELY ( retval == NULL ){
		if ( try_fdlimit() == 0 ){
			goto try_again;
		}
		else {	print_error_sys(errno, "fopen", pathname, fatality); }
	}
	return retval;
}

/* ------------------------------------------------------------------------ */

/**@fn try_fdlimit
 * @brief try to increase the file desrciptor limit if we have not already
 *
 * @returns 0 on success
**/
static int
try_fdlimit(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	static bool tried = false;

	if ( ! tried ){
		tried = true;
		fdlimit_check();
		return 0;
	}
	return -1;
}

/* ======================================================================== */

/**@fn openedfiles_add
 * @brief add a file to the opened files struct array
 *
 * @param of   - opened files struct array
 * @param name - name of the opened file (from argv)
 *
 * @return 0 on success, else errno
**/
BUILD NOINLINE int
openedfiles_add(
	struct OpenedFiles *const RESTRICT of,
	/*@dependent@*/ char *const RESTRICT name
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*of
@*/
/*@allocates	of->file,
		of->file[]
@*/
{
	int retval = 0;
	struct OpenedFilesMember **added;

	of->nmemb += 1u;
	of->file   = realloc_check(of->file, of->nmemb * (sizeof *of->file));

	 added = &of->file[of->nmemb - 1u];
	*added = calloc_check(SIZE_C(1), sizeof **added);

	(*added)->infile = fopen_check(name, "rb", NONFATAL);
	if ( (*added)->infile == NULL ){
		retval = errno;
	}
	(*added)->infile_name = name;

	return retval;
}

/**@fn openedfiles_close_free
 * @brief closes any files and frees any allocated pointer in the opened files
 *   array struct
 *
 * @param of - opened files struct
**/
BUILD NOINLINE void
openedfiles_close_free(struct OpenedFiles *const RESTRICT of)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
/*@releases	of->file,
		of->file[]
@*/
{
	size_t i;

	for ( i = 0; i < of->nmemb; ++i ){
		if ( of->file[i]->infile != NULL ){
			(void) fclose(of->file[i]->infile);
		}
		free(of->file[i]);
	}
	free(of->file);
	of->nmemb = 0;

	return;
}

/* ======================================================================== */

/**@fn filestats_get
 * @brief gets/set the file stats of an opened file
 *
 * @param ofm  - opened files struct array member
 * @param mode - encode or decode
 *
 * @return 0 on success, else number of errors
**/
BUILD NOINLINE unsigned int
filestats_get(
	struct OpenedFilesMember *const RESTRICT ofm,
	const enum ProgramMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		ofm->fstat
@*/
{
	unsigned int retval = 0;
	union {	int		d;
		enum FileCheck	fc;
	} result;

	/* bad filename check; would have already inc. error count */
	if ( ofm->infile == NULL ){
		return 0;
	}

	/* check for supported filetypes and fill most of fstat */
	if ( (mode == MODE_ENCODE) && g_flag.rawpcm ){
		rawpcm_statcopy(&ofm->fstat);
		/* MAYBE: mode for 'stdin' reading */
		result.d = fseeko(ofm->infile, 0, SEEK_END);
		if UNLIKELY ( result.d != 0 ){
			error_sys(errno, "fseeko", ofm->infile_name);
		}
		ofm->fstat.decpcm_off  = 0;
		ofm->fstat.decpcm_size = (size_t) ftello(ofm->infile);
	}
	else {	result.fc = filecheck_codecfmt(
			&ofm->fstat, ofm->infile, ofm->infile_name, mode
		);
		if ( result.fc != FILECHECK_OK ){
			return 1u;
		}
	}

	/* the rest of fstat */
	ofm->fstat.framelen    = libttaR_nsamples_perframe_tta1(
		ofm->fstat.samplerate
	);
	ofm->fstat.buflen      = (size_t) (
		ofm->fstat.framelen * ofm->fstat.nchan
	);
	ofm->fstat.samplebytes = (enum LibTTAr_SampleBytes) (
		(ofm->fstat.samplebits + 7u) / 8u
	);

	/* checks */
	if UNLIKELY (
		libttaR_test_nchan((unsigned int) ofm->fstat.nchan) == 0
	){
		error_tta_nf("%s: libttaR built without support for "
			"%"PRIu16" audio channels", ofm->infile_name,
			ofm->fstat.nchan
		);
		retval += 1u;;
	}
	if UNLIKELY ( ofm->fstat.framelen == 0 ){
		error_tta_nf("%s: samplerate of 0", ofm->infile_name);
		retval += 1u;
	}
	if UNLIKELY ( ofm->fstat.samplebytes == 0 ){
		error_tta_nf("%s: 0 bits per sample", ofm->infile_name);
		retval += 1u;
	}

	return retval;
}

/* ------------------------------------------------------------------------ */

/**@fn filecheck_codecfmt
 * @brief checks if the 'file' is a supported format
 *
 * @param fstat    - bloated file stats struct
 * @param file     - source file
 * @param filename - the name of the source file (errors)
 * @param mode     - encode or decode
 *
 * @return FILECHECK_OK on success
**/
static enum FileCheck
filecheck_codecfmt(
	/*@out@*/ struct FileStats *const RESTRICT fstat,
	FILE *const RESTRICT file, const char *const RESTRICT filename,
	const enum ProgramMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		fstat,
		file
@*/
{
	union {	enum FileCheck	fc; } result;

	/* seek past any metadata on the input file */
	result.fc = metatags_skip(file);
	if ( result.fc != FILECHECK_MISMATCH ){
		goto end_error;
	}

	switch ( mode ){
	case MODE_ENCODE:
		/* wav */
		result.fc = filecheck_wav(fstat, file);
		if ( result.fc == FILECHECK_OK ){
			break;
		}
		if ( result.fc != FILECHECK_MISMATCH ){
			goto end_error;
		}
		/* w64 */
		result.fc = filecheck_w64(fstat, file);
		if ( result.fc == FILECHECK_OK ){
			break;
		}
		goto end_error;
	case MODE_DECODE:
		/* tta1 */
		result.fc = filecheck_tta1(fstat, file);
		if ( result.fc == FILECHECK_OK ){
			break;
		}
		goto end_error;
	}

	/* check that file stats are within bounds / reasonable */
	if UNLIKELY (
	     (fstat->nchan == 0)
	    ||
	     (fstat->samplerate == 0)
	    ||
	     (fstat->samplebits == 0)
	    ||
	     (fstat->samplebits > (uint16_t) LIBTTAr_SAMPLEBITS_MAX)
	){
		result.fc = FILECHECK_UNSUPPORTED_RESOLUTION;
end_error:
		error_filecheck(result.fc, errno, fstat, filename);
	}
	return result.fc;
}

/* ======================================================================== */

/**@fn get_encfmt_sfx
 * @brief gets the suffix string for an encoded format
 *
 * @param fmt - format type
 *
 * @return pointer to the suffix string
**/
CONST
/*@observer@*/
BUILD const char *
get_encfmt_sfx(const enum EncFormat fmt)
/*@*/
{
	/*@observer@*/
	const char *const ext[] = xENCFMT_EXT_ARRAY;

	return ext[fmt];
}

/**@fn get_decfmt_sfx
 * @brief gets the suffix string for an decoded format
 *
 * @param fmt - format type
 *
 * @return pointer to the suffix string
**/
CONST
/*@observer@*/
BUILD const char *
get_decfmt_sfx(const enum DecFormat fmt)
/*@*/
{
	/*@observer@*/
	const char *const ext[] = DECFMT_EXT_ARRAY;

	return ext[fmt];
}

/**@fn get_outfile_name
 * @brief constructs the destination file name from the parameters and some
 *   global flags
 *
 * @param infile_name - name of the source file
 * @param sfx         - suffix/file-extension string
 *
 * @return the destination file name/path string
**/
/*@only@*/
BUILD NOINLINE char *
get_outfile_name(const char *const infile_name, const char *const sfx)
/*@globals	internalState,
		fileSystem
@*/
/*@modifies	internalState,
		fileSystem
@*/
{
	char *retval;
	const char *outfile_name;
	const char *outfile_dir  = NULL;
	const char *outfile_sfx  = NULL;

	if ( g_flag.outfile != NULL ){
		if ( g_flag.outfile_is_dir ){
			outfile_dir  = g_flag.outfile;
			outfile_name = infile_name;
			outfile_sfx  = sfx;
		}
		else {	outfile_name = g_flag.outfile; }
	}
	else {	outfile_name = infile_name;
		outfile_sfx = sfx;
	}
	retval = outfile_name_fmt(
		outfile_dir, outfile_name, outfile_sfx
	);
	return retval;
}

/* ------------------------------------------------------------------------ */

/**@fn outfile_name_fmt
 * @brief constructs the destination file name from the parameters
 *
 * @param outfile_dir - name of the destination directory or NULL
 * @param infile_name - name of the source file
 * @param sfx         - suffix/file-extension string or NULL
 *
 * @return destination file name/path string
**/
/*@only@*/
static char *
outfile_name_fmt(
	/*@null@*/ const char *const outfile_dir,
	const char *infile_name, /*@null@*/ const char *const suffix
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	char *retval;
	size_t in_len = strlen(infile_name);
	size_t dir_len = 0, suffix_len = 0;
	size_t base_len;
	char *fxdot = NULL;
	union {	uintptr_t p; } tmp;

	if ( outfile_dir != NULL ){
		dir_len = strlen(outfile_dir);

		/* remove any directory paths from infile_name */
		tmp.p = (uintptr_t) findrchar(
			infile_name, PATH_DELIM, in_len
		);
		if ( tmp.p != 0 ){
			tmp.p  += 1u;
			in_len -= (size_t) (
				tmp.p - ((uintptr_t) infile_name)
			);
			infile_name = (char *) tmp.p;
		}
	}
	if ( suffix != NULL ){
		suffix_len = strlen(suffix);
		fxdot = findrchar(infile_name, '.', in_len);
	}

	if ( fxdot == NULL ){
		base_len = in_len;
	}
	else {	base_len = (size_t) (fxdot - infile_name); }

	retval = malloc_check(dir_len + base_len + suffix_len + 1u);

	if ( outfile_dir != NULL ){
		(void) memcpy(retval, outfile_dir, dir_len);
	}
	(void) memcpy(&retval[dir_len], infile_name, base_len);
	if ( suffix != NULL ){
		(void) memcpy(
			&retval[dir_len + base_len], suffix, suffix_len
		);
	}
	retval[dir_len + base_len + suffix_len] = '\0';

	return retval;
}

/**@fn findrchar
 * @brief rewritten memrchr (memrchr is a GNU extension)
 *
 * @param s - input string
 * @param c - target character
 * @param n - size of 's'
 *
 * @return a pointer to last instance of 'c' in 's'
 * @retval NULL not found
**/
PURE
/*@temp@*/ /*@null@*/
static char *
findrchar(const char *const RESTRICT s, const char c, size_t n)
/*@*/
{
	while ( n-- != 0 ){
		if ( s[n] == c ){
			return (char *) &s[n];
		}
	}
	return NULL;
}

/* EOF //////////////////////////////////////////////////////////////////// */
