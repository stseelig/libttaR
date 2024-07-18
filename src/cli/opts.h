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
#include "optsget.h"	// enum OptDict

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
extern const struct OptDict encode_optdict[];

/*@unchecked@*/
extern const struct OptDict decode_optdict[];

//////////////////////////////////////////////////////////////////////////////

#undef fstat
extern void rawpcm_statcopy(/*@out@*/ struct FileStats *restrict fstat)
/*@modifies	*fstat@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
