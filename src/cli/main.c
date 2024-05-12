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

#include <signal.h>
#include <stdbool.h>	// true
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <unistd.h>

#include "../bits.h"
#include "../version.h"

#include "debug.h"
#include "cli.h"
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

extern int tta2enc(uint)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

extern int tta2dec(uint)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

//--------------------------------------------------------------------------//

static void sighand(enum HandledSignals)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/ /*@unused@*/
const uint ttaR_num_version = TTAr_NUM_VERSION;
/*@unchecked@*/ /*@unused@*/
const uint ttaR_num_version_major = TTAr_NUM_VERSION_MAJOR;
/*@unchecked@*/ /*@unused@*/
const uint ttaR_num_version_minor = TTAr_NUM_VERSION_MINOR;
/*@unchecked@*/ /*@unused@*/
const uint ttaR_num_version_revis = TTAr_NUM_VERSION_REVIS;

/*@unchecked@*/ /*@unused@*/ /*@observer@*/
const char ttaR_str_version[] = TTAr_STR_VERSION;

/*@unchecked@*/ /*@unused@*/ /*@observer@*/
const char ttaR_str_copyright[] = TTAr_STR_COPYRIGHT;

/*@unchecked@*/ /*@unused@*/ /*@observer@*/
const char ttaR_str_license[] = TTAr_STR_LICENSE;

//--------------------------------------------------------------------------//

/*@checkmod@*/
uint g_argc;

/*@checkmod@*/ /*@dependent@*/
char **g_argv;

/*@checkmod@*/
u8 g_nwarnings;

/*@-fullinitblock@*/
/*@checkmod@*/
struct GlobalFlags g_flag = {
	.decfmt = DECFMT_W64
};
/*@=fullinitblock@*/

// MAYBE cli opt to change
/*@checkmod@*/
size_t g_samplebuf_len = G_SAMPLEBUF_LEN_DEFAULT;

// TODO cli opt to change + multithreaded-mode flag
/*@checkmod@*/
uint g_nthreads = 16u;

// TODO cli opt
/*@checkmod@*/
uint g_framequeue_len = 0;

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
		warning_tta("failed to setup sighandler");
	}

	// these are saved for argument parsing in the modes
	g_argc = (uint) argc;
	g_argv = argv;

	// enter a mode
	if ( strcmp(argv[1], "encode") == 0 ){
		r = tta2enc(2u);
	}
	else if ( strcmp(argv[1], "decode") == 0 ){
		r = tta2dec(2u);
	}
	else if UNLIKELY ( true ) {
		error_tta_nf("bad mode '%s'", argv[1]);
print_main_help:
		errprint_program_intro();
		(void) fprintf(stderr,
			" ttaR encode --help\n"
			" ttaR decode --help\n"
		);
		r = EXIT_FAILURE;
	} else{;}

	return r;
}

//--------------------------------------------------------------------------//

// only async-signal-safe functions should be used ($ man signal-safe)
static void
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
