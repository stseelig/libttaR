#ifndef TTA_CLI_H
#define TTA_CLI_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// cli.h                                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <time.h>	// timespec, size_t

#include "../splint.h"

#include "formats.h"	// struct FileStats
#include "main.h"	// enum ProgramMode

//////////////////////////////////////////////////////////////////////////////

INLINE double timediff(
	register const struct timespec *const restrict,
	register const struct timespec *const restrict
)
/*@*/
;

//==========================================================================//

extern void errprint_program_intro(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

extern void errprint_stats_precodec(
	const struct FileStats *const restrict, const char *const restrict,
	const char *const restrict, enum ProgramMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

extern void errprint_stats_postcodec(
	const struct FileStats *const restrict,
	const struct EncStats *const restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

extern void errprint_runtime(double, size_t, enum ProgramMode)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

extern void errprint_spinner(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

//////////////////////////////////////////////////////////////////////////////

INLINE double
timediff(
	register const struct timespec *const restrict start,
	register const struct timespec *const restrict finish
)
/*@*/
{
	register double diffsec;
	register const double diffnsec = (
		((double) (finish->tv_nsec - start->tv_nsec)) / 1000000000.0
	);

	diffsec  = (double) finish->tv_sec - start->tv_sec;
	diffsec += diffnsec;
	return diffsec;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
