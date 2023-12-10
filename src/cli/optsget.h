#ifndef TTA_OPTSGET_H
#define TTA_OPTSGET_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// optsget.h                                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../bits.h"

#include "open.h"

//////////////////////////////////////////////////////////////////////////////

enum OptMode {
	OPTMODE_LONG,
	OPTMODE_SHORT
};

//////////////////////////////////////////////////////////////////////////////

struct OptDict {
	char	 *longopt;
	int	  shortopt;	// -1: none
	int	(*fn)(uint, char *, enum OptMode);
};

//////////////////////////////////////////////////////////////////////////////

#undef of
uint optargs_process(
	struct OpenedFiles *const restrict of, uint,
	const struct OptDict *const restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*of
@*/
;

extern void optsget_argcheck(uint, char *, uint)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
