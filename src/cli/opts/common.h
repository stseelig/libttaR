#ifndef TTA_OPTS_COMMON_H
#define TTA_OPTS_COMMON_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/common.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../../bits.h"

#include "../optsget.h"	// enum OptMode

//////////////////////////////////////////////////////////////////////////////

extern int opt_common_quiet(uint, uint, uint, char *const *, enum OptMode)
/*@globals	g_flag@*/
/*@modifies	g_flag.quiet@*/
;

extern int opt_common_single_threaded(
	uint, uint, uint, char *const *, enum OptMode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.threadmode@*/
;

extern int opt_common_multi_threaded(
	uint, uint, uint, char *const *, enum OptMode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.threadmode@*/
;

extern int opt_common_delete_src(
	uint, uint, uint, char *const *, enum OptMode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.delete_src@*/
;

#undef argv
extern int opt_common_threads(
	uint, uint, uint, char *const *argv, enum OptMode
)
/*@globals	fileSystem,
		internalState,
		g_flag,
		g_nthreads
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.threadmode,
		g_nthreads,
		**argv
@*/
;

#undef argv
extern int opt_common_outfile(
	uint, uint, uint, char *const *argv, enum OptMode
)
/*@globals	fileSystem,
		internalState,
		g_flag
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.outfile,
		g_flag.outfile_is_dir,
		**argv
@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
