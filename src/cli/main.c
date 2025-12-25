/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// main.c                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../libttaR.h"
#include "../version.h"

#include "./common.h"
#include "./debug.h"
#include "./help.h"
#include "./main.h"
#include "./system.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef argv
BUILD_EXTERN NOINLINE int mode_encode(
	unsigned int, unsigned int, char *const *argv
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
BUILD_EXTERN NOINLINE int mode_decode(
	unsigned int, unsigned int, char *const *argv
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
;

/* ======================================================================== */

static void atexit_cleanup(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

/* //////////////////////////////////////////////////////////////////////// */

/*@-redef@*/

/**@var ttaR_info
 * @brief program version, copyright, and license info
**/
/*@unchecked@*/
BUILD const struct LibTTAr_VersionInfo ttaR_info = {
	CLI_VERSION_NUM,
	CLI_VERSION_NUM_MAJOR,
	CLI_VERSION_NUM_MINOR,
	CLI_VERSION_NUM_REVIS,
	CLI_VERSION_STR_EXTRA,
	CLI_VERSION_STR_DATE,
	CLI_COPYRIGHT_STR,
	CLI_LICENSE_STR
};

/**@var g_progname
 * @brief name of the program
**/
/*@checkmod@*/ /*@temp@*/
BUILD const char *g_progname = NULL;

/**@var g_nwarnings
 * @brief number of warnings and errors; exit status
**/
/*@checkmod@*/
BUILD uint8_t g_nwarnings = 0;

/**@var g_flag
 * @brief global flags struct
**/
/*@-fullinitblock@*/
/*@checkmod@*/
BUILD struct GlobalFlags g_flag = {
	.threadmode = THREADMODE_UNSET,
	.decfmt     = DECFMT_W64
};
/*@=fullinitblock@*/

/**@var g_nthreads
 * @brief number of coder threads to use
**/
/*@checkmod@*/
BUILD unsigned int g_nthreads = 0;

/**@var g_rm_on_sigint
 * @brief name of the currently opened destination file for removal on a
 *   handled signal
**/
/*@checkmod@*/ /*@dependent@*/ /*@null@*/
BUILD char *g_rm_on_sigint = NULL;

/*@=redef@*/

/* //////////////////////////////////////////////////////////////////////// */

/**@fn main
 * @brief read a book
 *
 * @param argc - argument count
 * @param argv - argument vector
 *
 * @return program exit status; number of warnings and errors
**/
int
main(const int argc, char *const *const argv)
/*@globals	fileSystem,
		internalState,
		g_progname
@*/
/*@modifies	fileSystem,
		internalState,
		**argv,
		g_progname
@*/
{
	int retval = EXIT_FAILURE;
	UNUSED union {	int d; } result;

	/* saved for warning/error printing */
	g_progname = argv[0];

	/* no arguments */
	if UNLIKELY ( argc == 1 ){
		goto print_main_help;
	}

	/* signals */
	signals_setup();

	/* atexit */
	result.d = atexit(atexit_cleanup);
	assert(result.d == 0);

	/* enter a mode */
	if ( strcmp(argv[1u], "encode") == 0 ){
		retval = mode_encode(2u, (unsigned int) argc, argv);
	}
	else if ( strcmp(argv[1u], "decode") == 0 ){
		retval = mode_decode(2u, (unsigned int) argc, argv);
	}
	else {	error_tta_nf("bad mode '%s'", argv[1u]);
print_main_help:
		errprint_help_main();
	}
	return retval;
}

/* ------------------------------------------------------------------------ */

/**@fn atexit_cleanup
 * @brief removes any incomplete file(s) for an early exit on error
**/
static void
atexit_cleanup(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	union {	int d; } result;

	if ( g_rm_on_sigint != NULL ){
		result.d = remove(g_rm_on_sigint);
		if ( (result.d != 0) && (errno != EACCES) ){ /* /dev/null */
			error_sys_nf(errno, "remove", g_rm_on_sigint);
		}
	}
	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
