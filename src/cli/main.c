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

#include <errno.h>
#include <signal.h>
#include <stdbool.h>	// true
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "../bits.h"
#include "../libttaR.h"
#include "../version.h"

#include "debug.h"
#include "help.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////////

enum HandledSignals {
	HS_ABRT = SIGABRT,
	HS_HUP  = SIGHUP,
	HS_INT  = SIGINT,
	HS_QUIT = SIGQUIT,
	HS_TERM = SIGTERM
};

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

//--------------------------------------------------------------------------//

static COLD NORETURN void sighand(enum HandledSignals)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
const struct LibTTAr_VersionInfo ttaR_info = {
	TTAr_VERSION_NUM,
	TTAr_VERSION_NUM_MAJOR,
	TTAr_VERSION_NUM_MINOR,
	TTAr_VERSION_NUM_REVIS,
	TTAr_VERSION_STR_EXTRA,
	TTAr_VERSION_STR_DATE,
	TTAr_COPYRIGHT_STR,
	TTAr_LICENSE_STR
};

//--------------------------------------------------------------------------//

/*@checkmod@*/
uint g_argc;

/*@checkmod@*/ /*@temp@*/
char **g_argv;

/*@checkmod@*/
u8 g_nwarnings;

/*@-fullinitblock@*/
/*@checkmod@*/
struct GlobalFlags g_flag = {
	.threadmode = THREADMODE_UNSET,
	.decfmt     = DECFMT_W64
};
/*@=fullinitblock@*/

/*@checkmod@*/
uint g_nthreads = 0;

/*@checkmod@*/ /*@dependent@*/ /*@null@*/
char *g_rm_on_sigint = NULL;

//////////////////////////////////////////////////////////////////////////////

int
main(int argc, /*@dependent@*/ char **argv)
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
	int r;

	if UNLIKELY ( argc == 1 ){
		goto print_main_help;
	}

	// setup signals
	if UNLIKELY (
	     (signal((int) HS_ABRT, (void (*)(int)) sighand) == SIG_ERR)
	    ||
	     (signal((int) HS_HUP , (void (*)(int)) sighand) == SIG_ERR)
	    ||
	     (signal((int) HS_INT , (void (*)(int)) sighand) == SIG_ERR)
	    ||
	     (signal((int) HS_QUIT, (void (*)(int)) sighand) == SIG_ERR)
	    ||
	     (signal((int) HS_TERM, (void (*)(int)) sighand) == SIG_ERR)
	){
		error_sys_nf(errno, "signal", NULL);
	}

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
		r = EXIT_FAILURE;
	} else{;}

	return r;
}

//--------------------------------------------------------------------------//

// only async-signal-safe functions should be used ($ man signal-safe)
static COLD NORETURN void
sighand(enum HandledSignals signum)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	const char intro0[] = "\n" T_B_DEFAULT;
	const char intro1[] = ": " T_PURPLE;
	const char *signame;
	const char intro2[] = T_B_DEFAULT " ";
	const char action[] = ". ";
	const char outro[]  = T_PURPLE "!" T_RESET "\n";

	// ignore any more signals
	(void) signal((int) signum, SIG_IGN);

	(void) write(STDERR_FILENO, intro0, (sizeof intro0) - 1u);
	(void) write(STDERR_FILENO, g_argv[0], strlen(g_argv[0]));
	(void) write(STDERR_FILENO, intro1, (sizeof intro1) - 1u);
	switch ( signum ){
	case HS_ABRT:
		signame = "SIGABRT";
		break;
	case HS_HUP:
		signame = "SIGHUP";
		break;
	case HS_INT:
		signame = "SIGINT";
		break;
	case HS_QUIT:
		signame = "SIGQUIT";
		break;
	case HS_TERM:
		signame = "SIGTERM";
		break;
	}
	(void) write(STDERR_FILENO, signame, strlen(signame));
	(void) write(STDERR_FILENO, intro2, (sizeof intro2) - 1u);

	// remove any incomplete file(s)
	if ( g_rm_on_sigint != NULL ){
		(void) write(STDERR_FILENO, action, (sizeof action) - 1u);
		(void) unlink(g_rm_on_sigint);
	}

	(void) write(STDERR_FILENO, outro, (sizeof outro) - 1u);
	_exit((int) signum);
}

// EOF ///////////////////////////////////////////////////////////////////////
