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
#include <signal.h>
#include <stdbool.h>	// true
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/stat.h>
#include <unistd.h>

#include "../bits.h"
#include "../libttaR.h"
#include "../version.h"

#include "debug.h"
#include "help.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////////

extern int mode_encode(uint)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

extern int mode_decode(uint)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

//////////////////////////////////////////////////////////////////////////////

static void atexit_cleanup(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static NORETURN COLD void sighand_cleanup_exit(int)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errwrite_action_start(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errwrite_action_end(int result)
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

/**@var g_argc
 * @brief copy of argc from main
**/
/*@checkmod@*/
uint g_argc;

/**@var g_argv
 * @brief copy of argv from main
**/
/*@checkmod@*/ /*@temp@*/
char *const *g_argv;

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
 * @param argv[in] argument vector
 *
 * @return program exit status; number of warnings and errors
**/
int
main(const int argc, char *const *const argv)
/*@globals	fileSystem,
		internalState,
		g_argc,
		g_argv
@*/
/*@modifies	fileSystem,
		internalState,
		g_argc,
		g_argv
@*/
{
	int r = EXIT_FAILURE;
	struct sigaction sigact;
	union {	int d; } t;
#ifdef NDEBUG
	(void) t.d;	// gcc
#endif
	if UNLIKELY ( argc == 1 ){
		goto print_main_help;
	}

	// signals
	memset(&sigact, 0x00, sizeof sigact);
	sigact.sa_handler = sighand_cleanup_exit;
	t.d = sigfillset(&sigact.sa_mask);
	assert(t.d == 0);
	//
	t.d = sigaction(SIGABRT, &sigact, NULL);
	assert(t.d == 0);
	t.d = sigaction(SIGHUP , &sigact, NULL);
	assert(t.d == 0);
	t.d = sigaction(SIGINT , &sigact, NULL);
	assert(t.d == 0);
	t.d = sigaction(SIGQUIT, &sigact, NULL);
	assert(t.d == 0);
	t.d = sigaction(SIGTERM, &sigact, NULL);
	assert(t.d == 0);

	// atexit
	t.d = atexit(atexit_cleanup);
	assert(t.d == 0);

	// these are saved for argument parsing in the modes
	g_argc = (uint) argc;
	g_argv = argv;

	// enter a mode
	if ( strcmp(argv[1u], "encode") == 0 ){
		r = mode_encode(2u);
	}
	else if ( strcmp(argv[1u], "decode") == 0 ){
		r = mode_decode(2u);
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

/**@fn sighand_cleanup_exit
 * @brief signal handler that removes any incomplete file(s) then _exits
 *
 * @param signum signal number
 *
 * @note only async-signal-safe functions should be used (man 7 signal-safety)
**/
static NORETURN COLD void
sighand_cleanup_exit(const int signum)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char intro0[]       = "\n" T_B_DEFAULT;
	const char intro1[]       = ": " T_PURPLE;
	// strsignal may not be async-signal-safe, but we are just _exit-ing
	const char *const signame = strsignal(signum);
	const char intro2[]       = T_DEFAULT " ";
	const char outro[]        = T_PURPLE "!" T_RESET "\n";
	//
	union {	int d; } t;

	(void) write(STDERR_FILENO, intro0, (sizeof intro0) - 1u);
	(void) write(STDERR_FILENO, g_argv[0], strlen(g_argv[0]));
	(void) write(STDERR_FILENO, intro1, (sizeof intro1) - 1u);
	(void) write(STDERR_FILENO, signame, strlen(signame));
	(void) write(STDERR_FILENO, intro2, (sizeof intro2) - 1u);

	// remove any incomplete file(s)
	if ( g_rm_on_sigint != NULL ){
		errwrite_action_start();
		t.d = unlink(g_rm_on_sigint);
		if ( (t.d != 0) && (errno == EACCES) ){	// /dev/null
			t.d = 0;
		}
		errwrite_action_end(t.d);
	}

	(void) write(STDERR_FILENO, outro, (sizeof outro) - 1u);
	_exit(signum);
}

/**@fn errwrite_action_start
 * @brief writes a '?' to stderr
**/
static void
errwrite_action_start(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char act_start[] = "?";

	(void) write(STDERR_FILENO, act_start, (sizeof act_start) - 1u);
	return;
}

/**@fn errwrite_action_start
 * @brief overwrites action_start with the return status of the action
 *
 * @param result action return value
**/
static void
errwrite_action_end(int result)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char act_ok[]  = "\b. ";
	const char act_err[] = "\b" T_RED "x" T_DEFAULT " ";
	//
	const char *str;
	size_t size;

	if ( result == 0 ){
		str  = act_ok;
		size = (sizeof act_ok) - 1u;
	}
	else {	str  = act_err;
		size = (sizeof act_err) - 1u;
	}
	(void) write(STDERR_FILENO, str, size);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
