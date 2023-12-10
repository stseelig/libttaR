//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// open.c                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debug.h"
#include "open.h"

//////////////////////////////////////////////////////////////////////////////

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
	if ( of->file == NULL ){
		error_sys(errno, "reallocarray", strerror(errno), NULL);
	}
	assert(of->file != NULL);

	 added = &of->file[of->nmemb - 1u];
	*added = calloc((size_t) 1, sizeof **added);
	if ( *added == NULL ){
		error_sys(errno, "calloc", strerror(errno), NULL);
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

#ifndef NDEBUG
void
openedfiles_close_free(struct OpenedFiles *const restrict of)
/*@globals		fileSystem@*/
/*@modifies		*of@*/
/*@releases		of->file,
			of->file[]
@*/
/*@ensures isnull	of->file@*/
{
	size_t i;

	for ( i = 0; i < of->nmemb; ++i ){
		if ( of->file[i]->infile != NULL ){
			(void) fclose(of->file[i]->infile);
			of->file[i]->infile = NULL;
		}
		free(of->file[i]);
		of->file[i] = NULL;
	}
	assert(of->file[0] == NULL);
	free(of->file);
	of->file  = NULL;
	of->nmemb = 0;
	return;
}
#endif

//==========================================================================//

/*@only@*/
char *
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
	union {
		uintptr_t p;
	} t;

	if ( outfile_dir != NULL ){
		dir_len = strlen(outfile_dir);
		// remove any directory paths from infile_name
		t.p = (uintptr_t) memrchr(infile_name, (int) PATH_DELIM, in_len);
		if ( t.p != 0 ){
			infile_name = (char *) (t.p + 1u);
			in_len  = strlen(infile_name);
		}
	}
	if ( suffix != NULL ){
		suffix_len = strlen(suffix) + 1u;	// +1 for null
		fxdot = memrchr(infile_name, (int) '.', in_len);
	}

	if ( fxdot == NULL ){
		base_len = in_len;
	}
	else {	base_len = (size_t) (fxdot - infile_name); }

	r = malloc(dir_len + base_len + suffix_len);
	if ( r == NULL ){
		error_sys(errno, "malloc", strerror(errno), NULL);
	}
	assert(r != NULL);

	if ( outfile_dir != NULL ){
		(void) memcpy(r, outfile_dir, dir_len);
	}
	(void) memcpy(&r[dir_len], infile_name, base_len);
	if ( suffix != NULL ){
		(void) memcpy(&r[dir_len + base_len], suffix, suffix_len);
	}

	return r;
}

/*@observer@*/
const char *
get_outfile_sfx(enum DecFormat format)
/*@*/
{
	switch ( format ){
	case FORMAT_RAWPCM:
		return ".raw";
	case FORMAT_W64:
		return ".w64";
	case FORMAT_WAV:
		return ".wav";
	}
}

// EOF ///////////////////////////////////////////////////////////////////////
