#ifndef TTA_CLI_H
#define TTA_CLI_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// cli.h                                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <time.h>	// timespec, size_t

#include "../splint.h"

#include "formats.h"	// struct FileStats
#include "main.h"	// enum ProgramMode

//////////////////////////////////////////////////////////////////////////////

#define	SPINNER_FREQ	((size_t) 64u)

//////////////////////////////////////////////////////////////////////////////

INLINE PURE double timediff(
	const struct timespec *restrict, const struct timespec *restrict
)
/*@*/
;

//==========================================================================//

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

//////////////////////////////////////////////////////////////////////////////

/**@fn timediff
 * @brief turns some timespec structs into a double
 *
 * @param start the first time
 * @param finish the second time
 *
 * @return the difference in seconds
**/
INLINE PURE double
timediff(
	const struct timespec *const restrict start,
	const struct timespec *const restrict finish
)
/*@*/
{
	const double diff_sec  = (double) (
		finish->tv_sec - start->tv_sec
	);
	const double diff_nsec = (
		((double) (finish->tv_nsec - start->tv_nsec)) / 1000000000.0
	);
	return diff_sec + diff_nsec;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
