#ifndef TTA_OPTS_H
#define TTA_OPTS_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../bits.h"

#include "formats.h"		// struct fstat
#include "open.h"		// struct OpenedFiles

#include "opts/optsget.h"	// struct OptDict

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
extern const struct OptDict encode_optdict;

/*@unchecked@*/
extern const struct OptDict decode_optdict;

//////////////////////////////////////////////////////////////////////////////

// optsget.c

#undef of
#undef argv
extern uint optargs_process(
	struct OpenedFiles *restrict of, uint, uint, char *const *argv,
	const struct OptDict *restrict
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

//--------------------------------------------------------------------------//

// encode.c

#undef fstat
extern void rawpcm_statcopy(/*@out@*/ struct FileStats *restrict fstat)
/*@modifies	*fstat@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
