#ifndef TTA_OPTSGET_H
#define TTA_OPTSGET_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// optsget.h                                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
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
	/*@observer@*/ /*@null@*/
	char	 *longopt;
	int	  shortopt;	// -1: none
	/*@null@*/
	int	(*fn)(uint, char *, enum OptMode);
};

//////////////////////////////////////////////////////////////////////////////

#undef of
extern uint optargs_process(
	struct OpenedFiles *restrict of, uint, const struct OptDict *restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*of
@*/
;

extern void optsget_argcheck(uint, const char *restrict, uint)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
