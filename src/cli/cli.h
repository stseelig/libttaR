#ifndef H_TTA_CLI_H
#define H_TTA_CLI_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// cli.h                                                                    //
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
#include "./main.h"

/* //////////////////////////////////////////////////////////////////////// */

#define	SPINNER_FREQ	SIZE_C(64)

/* //////////////////////////////////////////////////////////////////////// */

BUILD_EXTERN NOINLINE void errprint_stats_precodec(
	const struct FileStats *RESTRICT, const char *RESTRICT,
	const char *RESTRICT, enum ProgramMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

BUILD_EXTERN NOINLINE void errprint_stats_postcodec(
	const struct FileStats *RESTRICT, const struct EncStats *RESTRICT
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

BUILD_EXTERN NOINLINE void errprint_runtime(double, size_t, enum ProgramMode)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

HOT
BUILD_EXTERN void errprint_spinner(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CLI_H */
