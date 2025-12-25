#ifndef H_TTA_OPEN_H
#define H_TTA_OPEN_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// open.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stddef.h>

#include "./common.h"
#include "./formats.h"

/* //////////////////////////////////////////////////////////////////////// */

struct OpenedFilesMember {
	/*@dependent@*/ /*@relnull@*/
	FILE			*infile;
	/*@dependent@*/
	char			*infile_name;	/* from argv, not allocated */
	struct FileStats	 fstat;
};
typedef /*@only@*/ struct OpenedFilesMember	*op_OpenedFilesMember;

struct OpenedFiles {
	size_t			 nmemb;
	/*@only@*/
	op_OpenedFilesMember	*file;
};

/* //////////////////////////////////////////////////////////////////////// */

/*@dependent@*/ /*@null@*/
BUILD_EXTERN FILE *fopen_check(
	const char *RESTRICT, const char *RESTRICT, enum Fatality
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

#undef of
BUILD_EXTERN NOINLINE int openedfiles_add(
	struct OpenedFiles *const RESTRICT of,
	/*@dependent@*/ char *const RESTRICT
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
BUILD_EXTERN NOINLINE void
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
;

#undef ofm
BUILD_EXTERN NOINLINE unsigned int filestats_get(
	struct OpenedFilesMember *const RESTRICT ofm, const enum ProgramMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		ofm->fstat
@*/
;

CONST
/*@observer@*/
BUILD_EXTERN const char *get_encfmt_sfx(enum EncFormat) /*@*/;

CONST
/*@observer@*/
BUILD_EXTERN const char *get_decfmt_sfx(enum DecFormat) /*@*/;

/*@only@*/
BUILD_EXTERN NOINLINE char *get_outfile_name(const char *, const char *)
/*@globals	internalState,
		fileSystem
@*/
/*@modifies	internalState,
		fileSystem
@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_OPEN_H */
