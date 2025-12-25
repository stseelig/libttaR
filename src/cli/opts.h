#ifndef H_TTA_OPTS_H
#define H_TTA_OPTS_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "./formats.h"
#include "./open.h"

#include "./opts/optsget.h"

/* //////////////////////////////////////////////////////////////////////// */

/*@-redef@*/

/*@unchecked@*/
BUILD_EXTERN const struct OptDict encode_optdict;

/*@unchecked@*/
BUILD_EXTERN const struct OptDict decode_optdict;

/*@=redef@*/

/* //////////////////////////////////////////////////////////////////////// */

/* optsget.c */

#undef of
#undef argv
BUILD_EXTERN NOINLINE unsigned int optargs_process(
	struct OpenedFiles *RESTRICT of, unsigned int, unsigned int,
	char *const *argv, const struct OptDict *RESTRICT
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*of,
		**argv
@*/
;

/* ------------------------------------------------------------------------ */

/* encode.c */

#undef fstat
BUILD_EXTERN void rawpcm_statcopy(/*@out@*/ struct FileStats *RESTRICT fstat)
/*@modifies	*fstat@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_OPTS_H */
