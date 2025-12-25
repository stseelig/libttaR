#ifndef H_TTA_OPTS_OPTSGET_H
#define H_TTA_OPTS_OPTSGET_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/optsget.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "../common.h"

/* //////////////////////////////////////////////////////////////////////// */

enum OptMode {
	OPTMODE_LONG,
	OPTMODE_SHORT
};

/* //////////////////////////////////////////////////////////////////////// */

typedef int (*x_optdict_fnptr)(
	unsigned int, unsigned int, unsigned int, char *const *, enum OptMode
);
typedef /*@observer@*/ x_optdict_fnptr	optdict_fnptr;

struct OptDict {
	unsigned int	nmemb;
	const char	**longopt;
	const int	*shortopt;	/* -1: none */
	optdict_fnptr	*fn;
};

/* //////////////////////////////////////////////////////////////////////// */

extern void optsget_argcheck(
	unsigned int, unsigned int, unsigned int, const char *RESTRICT
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_OPTS_OPTSGET_H */
