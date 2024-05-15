#ifndef TTA_OPEN_H
#define TTA_OPEN_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// open.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stddef.h>	// size_t

#include "formats.h"	// struct FileStats

//////////////////////////////////////////////////////////////////////////////

// MAYBE changes field names

struct OpenedFilesMember {
	/*@dependent@*/
	FILE			*infile;
	/*@dependent@*/
	char			*infile_name;	// from argv, not allocated
	struct FileStats	 fstat;
};
typedef /*@only@*/ struct OpenedFilesMember	*op_OpenedFilesMember;

struct OpenedFiles {
	size_t			 nmemb;
	/*@only@*/
	op_OpenedFilesMember	*file;
};

//////////////////////////////////////////////////////////////////////////////

/*@dependent@*/ /*@null@*/
extern FILE *fopen_check(const char *, const char *, enum Fatality)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;


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

#undef of
extern void
openedfiles_close_free(struct OpenedFiles *const restrict of)
/*@globals	fileSystem@*/
/*@modifies	*of@*/
/*@releases	of->file,
		of->file[]
@*/
;

/*@observer@*/
extern const char *get_encfmt_sfx(enum EncFormat) /*@*/;

/*@observer@*/
extern const char *get_decfmt_sfx(enum DecFormat) /*@*/;

/*@only@*/
extern char *get_outfile_name(const char *, const char *)
/*@globals	internalState,
		fileSystem
@*/
/*@modifies	internalState,
		fileSystem
@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
