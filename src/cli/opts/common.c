/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/common.c                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <ctype.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#include "../common.h"
#include "../debug.h"
#include "../formats.h"
#include "../system.h"

#include "./optsget.h"

/* //////////////////////////////////////////////////////////////////////// */


/**@fn opt_common_quiet
 * @brief sets only warnings and errors will to be printed
 *
 * @param optind0 - unused
 * @param optind1 - unused
 * @param argc    - unused
 * @param argv    - unused
 * @param mode    - unused
 *
 * @return 0
**/
BUILD int
opt_common_quiet(
	UNUSED const unsigned int optind0, UNUSED const unsigned int optind1,
	UNUSED const unsigned int argc, UNUSED char *const *const argv,
	UNUSED const enum OptMode mode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.quiet@*/
{
	g_flag.quiet = true;

	return 0;
}

/**@fn opt_common_single_threaded
 * @brief enables single-threaded coding
 *
 * @param optind0 - unused
 * @param optind1 - unused
 * @param argc    - unused
 * @param argv    - unused
 * @param mode    - unused
 *
 * @return 0
**/
BUILD int
opt_common_single_threaded(
	UNUSED const unsigned int optind0, UNUSED const unsigned int optind1,
	UNUSED const unsigned int argc, UNUSED char *const *const argv,
	UNUSED const enum OptMode mode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.threadmode@*/
{
	g_flag.threadmode = THREADMODE_SINGLE;

	return 0;
}

/**@fn opt_common_multi_threaded
 * @brief enables multi-threaded coding
 *
 * @param optind0 - unused
 * @param optind1 - unused
 * @param argc    - unused
 * @param argv    - unused
 * @param mode    - unused
 *
 * @return 0
**/
BUILD int
opt_common_multi_threaded(
	UNUSED const unsigned int optind0, UNUSED const unsigned int optind1,
	UNUSED const unsigned int argc, UNUSED char *const *const argv,
	UNUSED const enum OptMode mode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.threadmode@*/
{
	g_flag.threadmode = THREADMODE_MULTI;

	return 0;
}

/**@fn opt_common_delete_src
 * @brief enables the delete source files flag
 *
 * @param optind0 - unused
 * @param optind1 - unused
 * @param argc    - unused
 * @param argv    - unused
 * @param mode    - unused
 *
 * @return 0
**/
BUILD int
opt_common_delete_src(
	UNUSED const unsigned int optind0, UNUSED const unsigned int optind1,
	UNUSED const unsigned int argc, UNUSED char *const *const argv,
	UNUSED const enum OptMode mode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.delete_src@*/
{
	g_flag.delete_src = true;

	return 0;
}

/**@fn opt_common_threads
 * @brief sets the number of coder threads to use
 *
 * @param optind0 - index of  'argv'
 * @param optind1 - index of *'argv'
 * @param argc    - argument count from main()
 * @param argv    - argument vector from main()
 * @param mode    - short or long
 *
 * @return number of args used (long), or number of char's read (short)
**/
BUILD int
opt_common_threads(
	const unsigned int optind0, const unsigned int optind1,
	const unsigned int argc, char *const *const argv,
	const enum OptMode mode
)
/*@globals	fileSystem,
		internalState,
		g_flag,
		g_nthreads
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.threadmode,
		g_nthreads,
		**argv
@*/
{
	char *const opt = &argv[optind0][optind1];
	/* * */
	int retval   = 0;
	char *subopt = NULL;
	size_t i;
	union {	int	d; } result;
	union {	size_t	z; } tmp;

	switch ( mode ){
	case OPTMODE_SHORT:
		if ( opt[1u] == '\0' ){
			optsget_argcheck(optind0, argc, 1u, opt);
			subopt = argv[optind0 + 1u];
			retval = -1;
		}
		else {	subopt = &opt[1u];
			tmp.z = strlen(subopt);
			retval = 0;
			for ( i = 0; i < tmp.z; ++i ){
				if ( isdigit(subopt[i]) == 0 ){
					/*@loopbreak@*/ break;
				}
				retval += 1;
			}
		}
		break;
	case OPTMODE_LONG:
		(void) strtok(opt, "=");
		subopt = strtok(NULL, "");
		if UNLIKELY ( subopt == NULL ){
			error_tta("%s: missing argument", "--threads");
		}
		retval = 0;
		break;
	}
	assert(subopt != NULL);

	result.d = atoi(subopt);
	if UNLIKELY ( result.d <= 0 ){
		error_tta("%s: argument out of range: %d", "--threads",
			result.d
		);
	}

	g_nthreads        = (unsigned int) result.d;
	g_flag.threadmode = THREADMODE_MULTI;

	return retval;
}

/**@fn opt_common_outfile
 * @brief sets the destination filename or directory
 *
 * @param optind0 - index of  'argv'
 * @param optind1 - index of *'argv'
 * @param argc    - argument count from main()
 * @param argv    - argument vector from main()
 * @param mode    - short or long
 *
 * @return number of args used (long), or number of char's read (short)
**/
BUILD int
opt_common_outfile(
	const unsigned int optind0, const unsigned int optind1,
	const unsigned int argc, char *const *const argv,
	const enum OptMode mode
)
/*@globals	fileSystem,
		internalState,
		g_flag
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.outfile,
		g_flag.outfile_is_dir,
		**argv
@*/
{
	char *const opt = &argv[optind0][optind1];
	/* * */
	int retval   = 0;
	char *subopt = NULL;

	switch ( mode ){
	default:
		assert(false);
		break;
	case OPTMODE_SHORT:
		if ( opt[1u] == '\0' ){
			optsget_argcheck(optind0, argc, 1u, opt);
			subopt = argv[optind0 + 1u];
			retval = -1;
		}
		else {	subopt = &opt[1u];
			retval = (int) strlen(subopt);
		}
		break;
	case OPTMODE_LONG:
		(void) strtok(opt, "=");
		subopt = strtok(NULL, "");
		if UNLIKELY ( subopt == NULL ){
			error_tta("%s: missing argument", "--outfile");
		}
		retval = 0;
		break;
	}
	assert(subopt != NULL);

	/* check if directory */
	if ( subopt[strlen(subopt) - 1u] == PATH_DELIM ){
		g_flag.outfile_is_dir = true;
	}
	else {	g_flag.outfile_is_dir = false;}

	g_flag.outfile = subopt;

	return retval;
}

/* EOF //////////////////////////////////////////////////////////////////// */
