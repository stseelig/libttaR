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
//  '--' ends opt processing (filenames only from then on)                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "../bits.h"
#include "../splint.h"

#include "debug.h"
#include "main.h"
#include "open.h"
#include "optsget.h"

//////////////////////////////////////////////////////////////////////////////

#undef argv
static int optsget(
	uint, uint, char *const *argv, const struct OptDict *restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
;

#undef argv
/*@-globuse@*/	// called function pointers
static int shortoptsget(
	uint, uint, char *const *argv, const struct OptDict *restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
;
/*@=globuse@*/

#undef argv
/*@-globuse@*/	// called function pointers
static int longoptget(
	uint, uint, char *const *argv, const struct OptDict *restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
;
/*@=globuse@*/

//////////////////////////////////////////////////////////////////////////////

/**@fn optargs_process
 * @brief open files and/or process the command line arguments
 *
 * @param of[in out] the opened files struct array
 * @param optind index of 'argv'
 * @param argc the argument count from main()
 * @param argv[in out] the argument vector from main()
 * @param optdict[in] the option dictionary
 *
 * @return 0 on success, else number of errors
**/
uint
optargs_process(
	struct OpenedFiles *const restrict of, uint optind, const uint argc,
	char *const *const argv, const struct OptDict *const restrict optdict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*of,
		**argv
@*/
{
	uint r = 0;
	int optrv = 0;

	while ( optind < argc ){
		if ( (optrv >= 0) && (argv[optind][0] == '-') ){
			// opt
			optrv   = optsget(optind, argc, argv, optdict);
			optind += (optrv >= 0 ? optrv : -optrv);
		}
		else {	// filename
			r += (uint) (
				(bool) openedfiles_add(of, argv[optind])
			);
			++optind;
		}
	}
	return r;
}

/**@fn optsget
 * @brief process the command line arguments
 *
 * @param optind index of 'argv'
 * @param argc the argument count from main()
 * @param argv[in out] the argument vector from main()
 * @param optdict[in] the option dictionary
 *
 * @return number of args used; negative number used on stop processing opts
**/
static int
optsget(
	const uint optind, const uint argc, char *const *const argv,
	const struct OptDict *const restrict optdict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
{
	char *arg;
	int i;
	int optrv = 0;
	union {	char *s; } t;

	for ( i = 0; optind + i < argc; i += optrv + 1 ){
		arg = argv[optind + i];
		if ( arg[0] != '-' ){	// return at first non-opt
			break;
		}
		else if ( (arg[0] == '-') && (arg[1u] != '-') ){
			optrv = shortoptsget(optind + i, argc, argv, optdict);
			if UNLIKELY ( (optrv < 0) || (arg[1u] == '\0') ){
				error_tta("bad shortopt: -%c", (char) -optrv);
			}
		}
		else if ( (arg[0] == '-') && (arg[1u] == '-') ){
			if ( arg[2u] == '\0' ){	// "--" stops opt processing
				return -(++i);
			}
			optrv = longoptget(optind + i, argc, argv, optdict);
			if UNLIKELY ( optrv < 0 ){
				t.s = strtok(arg, "=");
				assert(t.s != NULL);
				error_tta("bad longopt: %s", t.s);
			}
		} else{;}
	}
	return i;
}

//--------------------------------------------------------------------------//

/**@fn shortoptsget
 * @brief process an entire string of short ("-xyz") command line arguments
 *
 * @param optind index of 'argv'
 * @param argc the argument count from main()
 * @param argv[in out] the argument vector from main()
 * @param optdict[in] the option dictionary
 *
 * @return number of args used, or if a bad opt, the bad opt negated
 *
 * @note shortopt function-pointer return:
 *    >0: number of chars in the opt used
 *    <0: number of args used
**/
static int
shortoptsget(
	const uint optind, const uint argc, char *const *const argv,
	const struct OptDict *const restrict optdict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
{
	const char *const opt = &argv[optind][1u];
	int optrv = 0;
	uint i, j;

	for ( i = 0; opt[i] != '\0'; i += optrv + 1 ){
		for ( j = 0; j < optdict->nmemb; ++j ){
			if ( (int) opt[i] == optdict->shortopt[j] ){
				optrv = optdict->fn[j](
					optind, i + 1u, argc, argv,
					OPTMODE_SHORT
				);
				if ( optrv < 0 ){ return -optrv; }// args used
				goto cont_outer_loop;	// onto next char
			}
		}
		return (int) -opt[i];	// shortopt not found
cont_outer_loop:
		;
	}
	return 0;
}

/**@fn longoptsget
 * @brief process the long ("--xyz") command line arguments
 *
 * @param optind index of 'argv'
 * @param argc the argument count from main()
 * @param argv[in out] the argument vector from main()
 * @param optdict[in] the option dictionary
 *
 * @return number of args used, or -1 on bad opt
**/
static int
longoptget(
	const uint optind, const uint argc, char *const *const argv,
	const struct OptDict *const restrict optdict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
{
	int optrv = -1;
	const char *const opt = &argv[optind][2u];
	const char *subopt;
	size_t size = SIZE_MAX;
	uint i;

	subopt = strchr(opt, '=');
	if ( subopt != NULL ){
		size = (size_t) (subopt - opt);
	}

	for ( i = 0; i < optdict->nmemb; ++i ){
		if ( strncmp(opt, optdict->longopt[i], size) == 0 ){
			optrv = optdict->fn[i](
				optind, 0, argc, argv, OPTMODE_LONG
			);
			break;
		}
	}
	return optrv;
}

//==========================================================================//

/**@fn optsget_argcheck
 * @brief checks that 'argv' is long enough for the current 'opt'
 *
 * @param optind the index of 'argv'
 * @param argc the argument count from main()
 * @param nargs the number of arguments the 'opt' has
 * @param opt[in] the option to check (for error message)
**/
void
optsget_argcheck(
	const uint optind, const uint argc, const uint nargs,
	const char *const restrict opt
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	if UNLIKELY ( optind + nargs >= argc ){
		error_tta("opt '%s' missing %u arg%s",
			opt, nargs, nargs > 1u ? "s" : ""
		);
	}
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
