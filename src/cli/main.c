#define TTA_MAIN_C
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// main.c                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdbool.h>	// true
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../bits.h"
#include "../libttaR.h"
#include "../version.h"

#include "debug.h"
#include "help.h"
#include "main.h"
#include "system.h"

//////////////////////////////////////////////////////////////////////////////

#undef argv
extern int mode_encode(uint, uint, char *const *argv)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
;

#undef argv
extern int mode_decode(uint, uint, char *const *argv)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
;

//////////////////////////////////////////////////////////////////////////////

static void atexit_cleanup(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@struct ttaR_info
 * @brief program version, copyright, and license info
**/
/*@unchecked@*/
const struct LibTTAr_VersionInfo ttaR_info = {
	CLI_VERSION_NUM,
	CLI_VERSION_NUM_MAJOR,
	CLI_VERSION_NUM_MINOR,
	CLI_VERSION_NUM_REVIS,
	CLI_VERSION_STR_EXTRA,
	CLI_VERSION_STR_DATE,
	CLI_COPYRIGHT_STR,
	CLI_LICENSE_STR
};

//--------------------------------------------------------------------------//

/**@var g_progname
 * @brief name of the program
**/
/*@checkmod@*/ /*@temp@*/
const char *g_progname;

/**@var g_nwarnings
 * @brief number of warnings and errors; exit status
**/
/*@checkmod@*/
u8 g_nwarnings;

/**@struct g_flag
 * @brief global flags struct
**/
/*@-fullinitblock@*/
/*@checkmod@*/
struct GlobalFlags g_flag = {
	.threadmode = THREADMODE_UNSET,
	.decfmt     = DECFMT_W64
};
/*@=fullinitblock@*/

/**@var g_nthreads
 * @brief number of coder threads to use
**/
/*@checkmod@*/
uint g_nthreads = 0;

/**@var g_rm_on_sigint
 * @brief name of the currently opened destination file for removal on a
 *   handled signal
**/
/*@checkmod@*/ /*@dependent@*/ /*@null@*/
char *g_rm_on_sigint = NULL;

//////////////////////////////////////////////////////////////////////////////

/**@fn main
 * @brief read a book
 *
 * @param argc argument count
 * @param argv[in out] argument vector
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
	int r = EXIT_FAILURE;
	UNUSED union {	int d; } t;

	// saved for warning/error printing
	g_progname = argv[0];

	// no arguments
	if UNLIKELY ( argc == 1 ){
		goto print_main_help;
	}

	// signals
	signals_setup();

	// atexit
	t.d = atexit(atexit_cleanup);
	assert(t.d == 0);

	// enter a mode
	if ( strcmp(argv[1u], "encode") == 0 ){
		r = mode_encode(2u, (uint) argc, argv);
	}
	else if ( strcmp(argv[1u], "decode") == 0 ){
		r = mode_decode(2u, (uint) argc, argv);
	}
	else if UNLIKELY ( true ) {
		error_tta_nf("bad mode '%s'", argv[1u]);
print_main_help:
		errprint_help_main();
	} else{;}

	return r;
}

//--------------------------------------------------------------------------//

/**@fn atexit_cleanup
 * @brief removes any incomplete file(s) for an early exit on error
**/
static void
atexit_cleanup(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	union {	int d; } t;
	if UNLIKELY ( g_rm_on_sigint != NULL ){
		t.d = remove(g_rm_on_sigint);
		if ( (t.d != 0) && (errno != EACCES) ){	// /dev/null
			error_sys_nf(errno, "remove", g_rm_on_sigint);
		}
	}
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
