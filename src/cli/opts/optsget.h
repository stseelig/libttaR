#ifndef TTA_OPTS_OPTSGET_H
#define TTA_OPTS_OPTSGET_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/optsget.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../../bits.h"

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

extern void optsget_argcheck(uint, uint, uint, const char *restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
