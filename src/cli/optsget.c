//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// optsget.c                                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//  Both longopts and shortopts are supported.                              //
//                                                                          //
//  Longopts are either '--opt' or '--opt=value0[,value1,...]'              //
//  Shortopts can be concatenated like :                                    //
//      '-a', '-ab', '-avalue0bvalue1',  '-a value0 -b value1'              //
//                                                                          //
//  Opt processing continues until the first non-opt or "--".               //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../bits.h"

#include "debug.h"
#include "main.h"
#include "open.h"
#include "optsget.h"

//////////////////////////////////////////////////////////////////////////////

static int optsget(uint, const struct OptDict *const restrict)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

/*@-globuse@*/	// called function pointers
static int shortoptsget(uint, const struct OptDict *const restrict)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;
/*@=globuse@*/

/*@-globuse@*/	// called function pointers
static int longoptget(uint, const struct OptDict *const restrict)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;
/*@=globuse@*/

//////////////////////////////////////////////////////////////////////////////

// returns 0 on success, or number of errors
uint
optargs_process(
	struct OpenedFiles *const restrict of, uint optind,
	const struct OptDict *const restrict optdict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*of
@*/
{
	uint r = 0;
	bool endopts = false;
	union {	int d; } t;

	while ( optind < g_argc ){
		if ( (! endopts) && (g_argv[optind][0] == '-') ){
			// opt
			t.d = optsget(optind, optdict);
			if ( t.d < 0 ){
				endopts = true;
				t.d = -t.d;
			}
			optind += t.d;
		}
		else {	// filename
			r += (uint) (
				(bool) openedfiles_add(of, g_argv[optind])
			);
			++optind;
		}
	}
	return r;
}

// returns number of args used; negative number used on stop processing opts
static int
optsget(uint optind, const struct OptDict *const restrict optdict)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	char *arg;
	int i;
	union {	char	*s;
		int	d;
	} t;

	for ( i = 0; optind + i < g_argc; ++i ){
		arg = g_argv[optind + i];
		if ( arg[0] != '-' ){	// return at first non-opt
			break;
		}
		else if ( (arg[0] == '-') && (arg[1] != '-') ){
			t.d = shortoptsget(optind + i, optdict);
			if UNLIKELY ( t.d < 0 ){
				error_tta("bad shortopt: -%c", (char) -t.d);
			}
			i += t.d;
		}
		else if ( (arg[0] == '-') && (arg[1] == '-') ){
			if ( arg[2] == '\0' ){	// "--" stops opt processing
				++i;
				return -i;
			}
			t.d = longoptget(optind + i, optdict);
			if UNLIKELY ( t.d < 0 ){
				t.s = strtok(arg, "=");
				assert(t.s != NULL);
				error_tta("bad longopt: %s", t.s);
			}
			i += t.d;
		} else{;}
	}
	return i;
}

//--------------------------------------------------------------------------//

// returns the number of args used, or if a bad opt, the bad opt negated
// shortopt function-pointer return:
//	>0: number of chars in the opt used
// 	<0: number of args used
static int
shortoptsget(uint optind, const struct OptDict *const restrict optdict)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	const char *const restrict opt = &g_argv[optind][1];
	uint i, j;
	union {	int d; } t;

	for ( i = 0; opt[i] != '\0'; ++i ){
		for ( j = 0; optdict[j].shortopt != 0; ++j ){
			if ( (int) opt[i] == optdict[j].shortopt ){
				t.d = optdict[j].fn(
					optind, &g_argv[optind][i + 1u],
					OPTMODE_SHORT
				);
				if ( t.d < 0 ){ return -t.d; }	// args used
				i += t.d;
				goto cont_outer_loop;	// onto next char
			}
		}
		return (int) -opt[i];	// shortopt not found
cont_outer_loop:
		continue;

	}
	return 0;
}

// processes an entire string of shortopts
// returns number of args used, or -1 if bad opt
static int
longoptget(uint optind, const struct OptDict *const restrict optdict)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	int r = -1;
	const char *const opt = &g_argv[optind][2];
	const char *subopt;
	size_t size = SIZE_MAX;
	uint i;

	subopt = strchr(opt, '=');
	if ( subopt != NULL ){
		size = (size_t) (subopt - opt);
	}

	for ( i = 0; optdict[i].longopt != NULL; ++i ){
		if ( strncmp(opt, optdict[i].longopt, size) == 0 ){
			r = (int) optdict[i].fn(
				optind, g_argv[optind], OPTMODE_LONG
			);
			break;
		}
	}

	return r;
}

//==========================================================================//

void
optsget_argcheck(uint optind, char *opt, uint nargs)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	if UNLIKELY ( optind + nargs >= g_argc ){
		error_tta("opt '%s' missing %u arg%s",
			opt, nargs, nargs > 1u ? "s" : ""
		);
	}
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
