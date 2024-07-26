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
	uint		nmemb;
	const char	**longopt;
	const int	*shortopt;	// -1: none
	int (*const *fn)(uint, uint, uint, char *const *, enum OptMode);
};

//////////////////////////////////////////////////////////////////////////////

#undef of
#undef argv
extern uint optargs_process(
	struct OpenedFiles *restrict of, uint, uint, char *const *argv,
	const struct OptDict *restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*of,
		**argv
@*/
;

extern void optsget_argcheck(uint, uint, uint, const char *restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
