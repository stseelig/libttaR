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

#include <stdbool.h>

#include "../bits.h"

#include "formats.h"
#include "optsget.h"	// enum OptMode

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
extern const struct OptDict tta2enc_optdict[];

/*@unchecked@*/
extern const struct OptDict tta2dec_optdict[];

//////////////////////////////////////////////////////////////////////////////

// common

extern int opt_common_delete_src(uint, char *, enum OptMode)
/*@globals	g_flag@*/
/*@modifies	g_flag.delete_src@*/
;

#undef opt
extern int opt_common_outfile(uint, char *opt, enum OptMode)
/*@globals	internalState,
		g_flag
@*/
/*@modifies	internalState,
		g_flag.outfile,
		g_flag.outfile_is_dir,
		*opt
@*/
;

extern int opt_common_quiet(uint, char *, enum OptMode)
/*@globals	g_flag@*/
/*@modifies	g_flag.quiet@*/
;

//==========================================================================//

// tta2enc

#undef fstat
extern void rawpcm_statcopy(struct FileStats *const restrict fstat)
/*@modifies	fstat@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
