//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/common.c                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <ctype.h>	// isdigit
#include <stdlib.h>	// atoi
#include <string.h>	// strtok

#include "../../bits.h"
#include "../../splint.h"

#include "../debug.h"
#include "../formats.h"
#include "../main.h"	// PATH_DELIM
#include "../opts.h"
#include "../optsget.h"

//////////////////////////////////////////////////////////////////////////////


/**@fn opt_common_quiet
 * @brief sets only warnings and errors will to be printed
 *
 * @param optind0 unused
 * @param optind1 unused
 * @param argc unused
 * @param argv unused
 * @param mode unused
 *
 * @return 0
**/
int
opt_common_quiet(
	UNUSED const uint optind0, UNUSED const uint optind1,
	UNUSED const uint argc, UNUSED char *const *const argv,
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
 * @param optind0 unused
 * @param optind1 unused
 * @param argc unused
 * @param argv unused
 * @param mode unused
 *
 * @return 0
**/
int
opt_common_single_threaded(
	UNUSED const uint optind0, UNUSED const uint optind1,
	UNUSED const uint argc, UNUSED char *const *const argv,
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
 * @param optind0 unused
 * @param optind1 unused
 * @param argc unused
 * @param argv unused
 * @param mode unused
 *
 * @return 0
**/
int
opt_common_multi_threaded(
	UNUSED const uint optind0, UNUSED const uint optind1,
	UNUSED const uint argc, UNUSED char *const *const argv,
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
 * @param optind0 unused
 * @param optind1 unused
 * @param argc unused
 * @param argv unused
 * @param mode unused
 *
 * @return 0
**/
int
opt_common_delete_src(
	UNUSED const uint optind0, UNUSED const uint optind1,
	UNUSED const uint argc, UNUSED char *const *const argv,
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
 * @param optind0 the index of  'argv'
 * @param optind1 the index of *'argv'
 * @param argc the argument count from main()
 * @param argv[in out] the argument vector from main()
 * @param mode short or long
 *
 * @return number of args used (long), or number of char's read (short)
**/
int
opt_common_threads(
	const uint optind0, const uint optind1, const uint argc,
	char *const *const argv, const enum OptMode mode
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
	int r;
	char *const opt = &argv[optind0][optind1];
	char *subopt;
	size_t i;
	union {	int	d;
		size_t	z;
	} t;

	switch ( mode ){
	case OPTMODE_SHORT:
		if ( opt[1u] == '\0' ){
			optsget_argcheck(optind0, argc, 1u, opt);
			subopt = argv[optind0 + 1u];
			r = -1;
		}
		else {	subopt = &opt[1u];
			t.z = strlen(subopt);
			r = 0;
			for ( i = 0; i < t.z; ++i ){
				if ( isdigit(subopt[i]) == 0 ){
					/*@loopbreak@*/ break;
				}
				++r;
			}
		}
		break;
	case OPTMODE_LONG:
		(void) strtok(opt, "=");
		subopt = strtok(NULL, "");
		if UNLIKELY ( subopt == NULL ){
			error_tta("%s: missing argument", "--threads");
		}
		r = 0;
		break;
	}
	assert(subopt != NULL);

	t.d = atoi(subopt);
	if UNLIKELY ( t.d <= 0 ){
		error_tta("%s: argument out of range: %d", "--threads", t.d);
	}

	g_nthreads = (uint) t.d;
	g_flag.threadmode = THREADMODE_MULTI;

	return r;
}

/**@fn opt_common_outfile
 * @brief sets the destination filename or directory
 *
 * @param optind0 the index of  'argv'
 * @param optind1 the index of *'argv'
 * @param argc the argument count from main()
 * @param argv[in out] the argument vector from main()
 * @param mode short or long
 *
 * @return number of args used (long), or number of char's read (short)
**/
int
opt_common_outfile(
	const uint optind0, const uint optind1, const uint argc,
	char *const *const argv, const enum OptMode mode
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
	int r;
	char *const opt = &argv[optind0][optind1];
	char *subopt;

	switch ( mode ){
	case OPTMODE_SHORT:
		if ( opt[1u] == '\0' ){
			optsget_argcheck(optind0, argc, 1u, opt);
			subopt = argv[optind0 + 1u];
			r = -1;
		}
		else {	subopt = &opt[1u];
			r = (int) strlen(subopt);
		}
		break;
	case OPTMODE_LONG:
		(void) strtok(opt, "=");
		subopt = strtok(NULL, "");
		if UNLIKELY ( subopt == NULL ){
			error_tta("%s: missing argument", "--outfile");
		}
		r = 0;
		break;
	}
	assert(subopt != NULL);

	// check if directory
	if ( subopt[strlen(subopt) - 1u] == PATH_DELIM ){
		g_flag.outfile_is_dir = true;
	}
	else {	g_flag.outfile_is_dir = false;}

	g_flag.outfile = subopt;
	return r;
}

// EOF ///////////////////////////////////////////////////////////////////////
