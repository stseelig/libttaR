#ifndef TTA_CLI_H
#define TTA_CLI_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// cli.h                                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef S_SPLINT_S
#include "../splint.h"
#endif

/* ------------------------------------------------------------------------ */

#include <stddef.h>	// size_t

#include "formats.h"	// struct FileStats
#include "main.h"	// enum ProgramMode

//////////////////////////////////////////////////////////////////////////////

#define	SPINNER_FREQ	((size_t) 64u)

//////////////////////////////////////////////////////////////////////////////

extern void errprint_stats_precodec(
	const struct FileStats *restrict, const char *restrict,
	const char *restrict, enum ProgramMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

extern void errprint_stats_postcodec(
	const struct FileStats *restrict, const struct EncStats *restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

extern void errprint_runtime(double, size_t, enum ProgramMode)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

extern HOT void errprint_spinner(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
