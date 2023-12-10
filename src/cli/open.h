#ifndef TTA_OPEN_H
#define TTA_OPEN_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// open.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "formats.h"	// struct FileStats

//////////////////////////////////////////////////////////////////////////////

struct OpenedFilesMember {
	/*@dependent@*/ /*@relnull@*/
	FILE			*infile;
	/*@dependent@*/
	char			*infile_name;	// from argv, not allocated
	struct FileStats	 fstat;
};
typedef /*@only@*/ struct OpenedFilesMember	*op_OpenedFilesMember;

struct OpenedFiles {
	size_t			 nmemb;
	/*@only@*/ /*@relnull@*/
	op_OpenedFilesMember	*file;
};

//////////////////////////////////////////////////////////////////////////////

#undef of
extern int openedfiles_add(
	struct OpenedFiles *const restrict of,
	/*@dependent@*/ char *const restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	internalState,
		*of
@*/
/*@allocates	of->file,
		of->file[]
@*/
;

#ifndef NDEBUG
#undef of
extern void
openedfiles_close_free(struct OpenedFiles *const restrict of)
/*@globals		fileSystem@*/
/*@modifies		*of@*/
/*@releases		of->file,
			of->file[]
@*/
/*@ensures isnull	of->file@*/
;
#endif

/*@only@*/
extern char *outfile_name_fmt(
	/*@null@*/ const char *, const char *, /*@null@*/ const char *
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

/*@observer@*/
extern const char *get_outfile_sfx(enum DecFormat)
/*@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
