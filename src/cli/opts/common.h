#ifndef H_TTA_OPTS_COMMON_H
#define H_TTA_OPTS_COMMON_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/common.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "../common.h"

#include "./optsget.h"

/* //////////////////////////////////////////////////////////////////////// */

BUILD_EXTERN int opt_common_quiet(
	unsigned int, unsigned int, unsigned int, char *const *, enum OptMode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.quiet@*/
;

BUILD_EXTERN int opt_common_single_threaded(
	unsigned int, unsigned int, unsigned int, char *const *, enum OptMode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.threadmode@*/
;

BUILD_EXTERN int opt_common_multi_threaded(
	unsigned int, unsigned int, unsigned int, char *const *, enum OptMode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.threadmode@*/
;

BUILD_EXTERN int opt_common_delete_src(
	unsigned int, unsigned int, unsigned int, char *const *, enum OptMode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.delete_src@*/
;

#undef argv
BUILD_EXTERN int opt_common_threads(
	unsigned int, unsigned int, unsigned int, char *const *argv,
	enum OptMode
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
BUILD_EXTERN int opt_common_outfile(
	unsigned int, unsigned int, unsigned int, char *const *argv,
	enum OptMode
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

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_OPTS_COMMON_H */
