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

#include "formats.h"
#include "optsget.h"	// enum OptMode

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
extern const struct OptDict encode_optdict[];

/*@unchecked@*/
extern const struct OptDict decode_optdict[];

//////////////////////////////////////////////////////////////////////////////

// common

extern int opt_common_single_threaded(uint, char *, enum OptMode)
/*@globals	g_flag@*/
/*@modifies	g_flag.threadmode@*/
;

extern int opt_common_multi_threaded(uint, char *, enum OptMode)
/*@globals	g_flag@*/
/*@modifies	g_flag.threadmode@*/
;

extern int opt_common_low_memory(uint, char *, enum OptMode)
/*@globals	g_flag@*/
/*@modifies	g_flag.threadmode@*/
;

extern int opt_common_delete_src(uint, char *, enum OptMode)
/*@globals	g_flag@*/
/*@modifies	g_flag.delete_src@*/
;

#undef opt
extern int opt_common_outfile(uint, char *opt, enum OptMode)
/*@globals	fileSystem,
		internalState,
		g_flag
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.outfile,
		g_flag.outfile_is_dir,
		*opt
@*/
;

extern int opt_common_quiet(uint, char *, enum OptMode)
/*@globals	g_flag@*/
/*@modifies	g_flag.quiet@*/
;

#undef opt
extern int opt_common_threads(uint, char *opt, enum OptMode)
/*@globals	fileSystem,
		internalState,
		g_flag,
		g_nthreads
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.threadmode,
		g_nthreads,
		*opt
@*/
;

//==========================================================================//

// tta2enc

#undef fstat
extern void rawpcm_statcopy(/*@out@*/ struct FileStats *restrict fstat)
/*@modifies	*fstat@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
