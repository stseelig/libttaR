//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// open.c                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdint.h>	// uintptr_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/resource.h>

#include "../libttaR.h"

#include "debug.h"
#include "main.h"
#include "open.h"
#include "opts.h"

//////////////////////////////////////////////////////////////////////////////

static bool try_fdlimit(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

static void fdlimit_check(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

//--------------------------------------------------------------------------//

#undef fstat
#undef file
static enum FileCheck filecheck_codecfmt(
	struct FileStats *const restrict fstat, FILE *const restrict file,
	const char *const restrict, const enum ProgramMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		fstat,
		file
@*/
;

//--------------------------------------------------------------------------//

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

//////////////////////////////////////////////////////////////////////////////

/*@dependent@*/ /*@null@*/
FILE
*fopen_check(const char *pathname, const char *mode, enum Fatality fatality)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	FILE *r;

try_again:
	r = fopen(pathname, mode);
	if UNLIKELY ( r == NULL ){
		if ( try_fdlimit() ){ goto try_again; }
		else {	print_error_sys(errno, "fopen", pathname, fatality); }
	}
	return r;
}

//--------------------------------------------------------------------------//

// true:  continue
// false: fallthrough
static bool
try_fdlimit(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	static bool tried;

	if ( ! tried ){
		fdlimit_check();
		tried = true;
		return true;
	}
	return false;
}

// attempt to increase the open-file limit
static void
fdlimit_check(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	struct rlimit limit;
	union {	int d; } t;

	t.d = getrlimit((int) RLIMIT_NOFILE, &limit);
	if UNLIKELY ( t.d != 0 ){
		error_sys(errno, "getrlimit", NULL);
	}

	limit.rlim_cur = limit.rlim_max;

	t.d = setrlimit((int) RLIMIT_NOFILE, &limit);
	if UNLIKELY ( t.d != 0 ){
		error_sys(errno, "setrlimit", NULL);
	}

	return;
}

//==========================================================================//

// returns errno, if any
int
openedfiles_add(
	struct OpenedFiles *const restrict of,
	/*@dependent@*/ char *const restrict name
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
	int r = 0;
	struct OpenedFilesMember **added;

	++(of->nmemb);
	of->file = reallocarray(of->file, of->nmemb, sizeof *of->file);
	if UNLIKELY ( of->file == NULL ){
		error_sys(errno, "reallocarray", NULL);
	}
	assert(of->file != NULL);

	 added = &of->file[of->nmemb - 1u];
	*added = calloc((size_t) 1u, sizeof **added);
	if UNLIKELY ( *added == NULL ){
		error_sys(errno, "calloc", NULL);
	}
	assert(*added != NULL);
	assert(of->file[of->nmemb - 1u] != NULL);

	(*added)->infile = fopen_check(name, "r", NONFATAL);
	if ( (*added)->infile == NULL ){
		r = errno;
	}
	(*added)->infile_name = name;

	return r;
}

void
openedfiles_close_free(struct OpenedFiles *const restrict of)
/*@globals	fileSystem@*/
/*@modifies	*of@*/
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

//==========================================================================//

// returns 0 on success, or number of errors
uint
filestats_get(
	struct OpenedFilesMember *const restrict ofm,
	const enum ProgramMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		ofm->fstat
@*/
{
	union {	int		d;
		enum FileCheck	fc;
	} t;

	// bad filename check; would have already inc. error count
	if ( ofm->infile == NULL ){ return 0; }

	// check for supported filetypes and fill most of fstat
	if ( (mode == MODE_ENCODE) && g_flag.rawpcm ){
		rawpcm_statcopy(&ofm->fstat);
		// TODO mode for stdin reading
		t.d = fseeko(ofm->infile, 0, SEEK_END);
		if UNLIKELY ( t.d != 0 ){
			error_sys(errno, "fseeko", ofm->infile_name);
		}
		ofm->fstat.decpcm_off  = 0;
		ofm->fstat.decpcm_size = (size_t) ftello(ofm->infile);
	}
	else {	t.fc = filecheck_codecfmt(
			&ofm->fstat, ofm->infile, ofm->infile_name, mode
		);
		if ( t.fc != FILECHECK_OK ){ return 1u; }
	}

	if UNLIKELY ( ! libttaR_test_nchan((uint)ofm->fstat.nchan) ){
		error_tta_nf("%s: libttaR built without support for "
			"%u audio channels", ofm->infile_name,
			ofm->fstat.nchan
		);
		return 1u;
	}

	// the rest of fstat
	ofm->fstat.framelen    = libttaR_nsamples_perframe_tta1(
		ofm->fstat.samplerate
	);
	ofm->fstat.buflen      = (size_t) (
		ofm->fstat.framelen * ofm->fstat.nchan
	);
	ofm->fstat.samplebytes = (enum TTASampleBytes) (
		(ofm->fstat.samplebits + 7u) / 8u
	);

	return 0;
}

//--------------------------------------------------------------------------//

static enum FileCheck
filecheck_codecfmt(
	struct FileStats *const restrict fstat, FILE *const restrict file,
	const char *const restrict filename, const enum ProgramMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		fstat,
		file
@*/
{
	union {	enum FileCheck	fc; } t;

	// seek past any metadata on the input file
	t.fc = metatags_skip(file);
	if ( t.fc != FILECHECK_MISMATCH ){ goto end_error; }

	switch ( mode ){
	case MODE_ENCODE:
		// wav
		t.fc = filecheck_wav(fstat, file);
		if ( t.fc == FILECHECK_OK ){ break; }
		if ( t.fc != FILECHECK_MISMATCH ){ goto end_error; }
		// w64
		t.fc = filecheck_w64(fstat, file);
		if ( t.fc == FILECHECK_OK ){ break; }
		goto end_error;
	case MODE_DECODE:
		// tta1
		t.fc = filecheck_tta1(fstat, file);
		if ( t.fc == FILECHECK_OK ){ break; }
		goto end_error;
	}

	// check that file stats are within bounds / reasonable
	if UNLIKELY (
	     (fstat->nchan == 0)
	    ||
	     (fstat->samplerate == 0)
	    ||
	     (fstat->samplebits == 0)
	    ||
	     (fstat->samplebits > (u16) TTA_SAMPLEBITS_MAX)
	){
		t.fc = FILECHECK_UNSUPPORTED_RESOLUTION;
end_error:
		error_filecheck(t.fc, fstat, filename, errno);
	}
	return t.fc;
}

//==========================================================================//

/*@observer@*/
CONST const char *
get_encfmt_sfx(enum EncFormat fmt)
/*@*/
{
	/*@observer@*/
	const char *const ext[] = xENCFMT_EXT_ARRAY;
	return ext[fmt];
}

/*@observer@*/
CONST const char *
get_decfmt_sfx(enum DecFormat fmt)
/*@*/
{
	/*@observer@*/
	const char *const ext[] = DECFMT_EXT_ARRAY;
	return ext[fmt];
}

/*@only@*/
char *
get_outfile_name(const char *infile_name, const char *sfx)
/*@globals	internalState,
		fileSystem
@*/
/*@modifies	internalState,
		fileSystem
@*/
{
	char *r;
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
	r = outfile_name_fmt(
		outfile_dir, outfile_name, outfile_sfx
	);

	return r;
}

//--------------------------------------------------------------------------//

/*@only@*/
static char *
outfile_name_fmt(
	/*@null@*/ const char *outfile_dir, const char *infile_name,
	/*@null@*/ const char *suffix
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	char *r;
	size_t in_len = strlen(infile_name);
	size_t dir_len = 0, suffix_len = 0;
	size_t base_len;
	char *fxdot = NULL;
	union {	uintptr_t p; } t;

	if ( outfile_dir != NULL ){
		dir_len = strlen(outfile_dir);
		// remove any directory paths from infile_name
		t.p = (uintptr_t) memrchr(
			infile_name, (int) PATH_DELIM, in_len
		);
		if ( t.p != 0 ){
			++t.p;
			in_len -= (size_t) (t.p - ((uintptr_t) infile_name));
			infile_name = (char *) t.p;
		}
	}
	if ( suffix != NULL ){
		suffix_len = strlen(suffix);
		fxdot = memrchr(infile_name, (int) '.', in_len);
	}

	if ( fxdot == NULL ){
		base_len = in_len;
	}
	else {	base_len = (size_t) (fxdot - infile_name); }

	r = malloc(dir_len + base_len + suffix_len + 1u);
	if UNLIKELY ( r == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(r != NULL);

	if ( outfile_dir != NULL ){
		(void) memcpy(r, outfile_dir, dir_len);
	}
	(void) memcpy(&r[dir_len], infile_name, base_len);
	if ( suffix != NULL ){
		(void) memcpy(&r[dir_len + base_len], suffix, suffix_len);
	}
	r[dir_len + base_len + suffix_len] = '\0';

	return r;
}

// EOF ///////////////////////////////////////////////////////////////////////
