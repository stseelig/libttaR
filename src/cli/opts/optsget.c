/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/optsget.c                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
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
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "../common.h"
#include "../debug.h"
#include "../main.h"
#include "../open.h"

#include "./optsget.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef argv
static int optsget(
	unsigned int, unsigned int, char *const *argv,
	const struct OptDict *RESTRICT
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
/*@-globuse@*/	/* called function pointers */
static int shortoptsget(
	unsigned int, unsigned int, char *const *argv,
	const struct OptDict *RESTRICT
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
/*@-globuse@*/	/* called function pointers */
static int longoptget(
	unsigned int, unsigned int, char *const *argv,
	const struct OptDict *RESTRICT
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

/* //////////////////////////////////////////////////////////////////////// */

/**@fn optargs_process
 * @brief open files and/or process the command line arguments
 *
 * @param of      - opened files struct array
 * @param optind  - index of 'argv'
 * @param argc    - argument count from main()
 * @param argv    - argument vector from main()
 * @param optdict - option dictionary
 *
 * @return 0 on success, else number of errors
**/
BUILD NOINLINE unsigned int
optargs_process(
	struct OpenedFiles *const RESTRICT of, unsigned int optind,
	const unsigned int argc, char *const *const argv,
	const struct OptDict *const RESTRICT optdict
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
	unsigned int retval = 0;
	int  optrv  = 0;

	while ( optind < argc ){
		if ( (optrv >= 0) && (argv[optind][0] == '-') ){
			/* opt */
			optrv   = optsget(optind, argc, argv, optdict);
			optind += (optrv >= 0 ? optrv : -optrv);
		}
		else {	/* filename */
			retval += (uint8_t) (
				openedfiles_add(of, argv[optind]) != 0
			);
			optind += 1u;
		}
	}
	return retval;
}

/**@fn optsget
 * @brief process the command line arguments
 *
 * @param optind  - index of 'argv'
 * @param argc    - argument count from main()
 * @param argv    - argument vector from main()
 * @param optdict - option dictionary
 *
 * @return number of args used; negative number used on stop processing opts
**/
static int
optsget(
	const unsigned int optind, const unsigned int argc,
	char *const *const argv, const struct OptDict *const RESTRICT optdict
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
	int optrv = 0;
	int i;
	union {	char *s; } tmp;

	for ( i = 0; optind + i < argc; i += optrv + 1 ){
		arg = argv[optind + i];
		if ( arg[0] != '-' ){	/* return at first non-opt */
			break;
		}
		else if ( (arg[0] == '-') && (arg[1u] != '-') ){
			optrv = shortoptsget(optind + i, argc, argv, optdict);
			if UNLIKELY ( (optrv < 0) || (arg[1u] == '\0') ){
				error_tta("bad shortopt: -%c", (char) -optrv);
			}
		}
		else if ( (arg[0] == '-') && (arg[1u] == '-') ){
			/* "--" ends opt processing */
			if ( arg[2u] == '\0' ){
				return -(i + 1u);
			}
			optrv = longoptget(optind + i, argc, argv, optdict);
			if UNLIKELY ( optrv < 0 ){
				tmp.s = strtok(arg, "=");
				assert(tmp.s != NULL);
				error_tta("bad longopt: %s", tmp.s);
			}
		} else{;}
	}
	return i;
}

/* ------------------------------------------------------------------------ */

/**@fn shortoptsget
 * @brief process an entire string of short ("-xyz") command line arguments
 *
 * @param optind  - index of 'argv'
 * @param argc    - argument count from main()
 * @param argv    - argument vector from main()
 * @param optdict - option dictionary
 *
 * @return number of args used, or if a bad opt, the bad opt negated
 *
 * @note shortopt function-pointer return:
 *    >0: number of chars in the opt used
 *    <0: number of args used
**/
static int
shortoptsget(
	const unsigned int optind, const unsigned int argc,
	char *const *const argv, const struct OptDict *const RESTRICT optdict
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
	/* * */
	int optrv = 0;
	unsigned int i, j;

	for ( i = 0; opt[i] != '\0'; i += optrv + 1 ){
		for ( j = 0; j < optdict->nmemb; ++j ){
			if ( (int) opt[i] == optdict->shortopt[j] ){
				optrv = optdict->fn[j](
					optind, i + 1u, argc, argv,
					OPTMODE_SHORT
				);
				if ( optrv < 0 ){
					return -optrv;	/* args used */
				}
				goto cont_outer_loop;	/* onto next char */
			}
		}
		return (int) -opt[i];	/* shortopt not found */
cont_outer_loop:
		;
	}
	return 0;
}

/**@fn longoptsget
 * @brief process the long ("--xyz") command line arguments
 *
 * @param optind  - index of 'argv'
 * @param argc    - argument count from main()
 * @param argv    - argument vector from main()
 * @param optdict - option dictionary
 *
 * @return number of args used, or -1 on bad opt
**/
static int
longoptget(
	const unsigned int optind, const unsigned int argc,
	char *const *const argv, const struct OptDict *const RESTRICT optdict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
{
	const char *const opt = &argv[optind][2u];
	/* * */
	int optrv = -1;
	const char *subopt;
	size_t size = SIZE_MAX;
	unsigned int i;

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

/* ======================================================================== */

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
	const unsigned int optind, const unsigned int argc,
	const unsigned int nargs, const char *const RESTRICT opt
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

/* EOF //////////////////////////////////////////////////////////////////// */
