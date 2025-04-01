#ifndef TTA_SYSTEM_POSIX_H
#define TTA_SYSTEM_POSIX_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// system.posix.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <sys/resource.h>
#include <time.h>
#include <unistd.h>

#include "../bits.h"

#include "debug.h"
#include "main.h"	// g_progname

//////////////////////////////////////////////////////////////////////////////

#define PATH_DELIM	'/'

typedef struct timespec	timestamp_p;

//////////////////////////////////////////////////////////////////////////////

/*@unused@*/
static NORETURN COLD void sighand_cleanup_exit(int)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

/*@unused@*/
static void errwrite_action_start(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

/*@unused@*/
static void errwrite_action_end(int)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

//////////////////////////////////////////////////////////////////////////////

/// @see "system.h"
INLINE void
signals_setup(void)
/*@globals	internalState@*/
/*@modifies	internalState@*/
{
	struct sigaction sigact;

	UNUSED union {	int d; } result;

	memset(&sigact, 0x00, sizeof sigact);
	sigact.sa_handler = sighand_cleanup_exit;
	result.d = sigfillset(&sigact.sa_mask);
	assert(result.d == 0);

	result.d = sigaction(SIGABRT, &sigact, NULL);
	assert(result.d == 0);
	result.d = sigaction(SIGHUP , &sigact, NULL);
	assert(result.d == 0);
	result.d = sigaction(SIGINT , &sigact, NULL);
	assert(result.d == 0);
	result.d = sigaction(SIGQUIT, &sigact, NULL);
	assert(result.d == 0);
	result.d = sigaction(SIGTERM, &sigact, NULL);
	assert(result.d == 0);

	return;
}

//--------------------------------------------------------------------------//

/**@fn sighand_cleanup_exit
 * @brief signal handler that removes any incomplete file(s) then _exits
 *
 * @param signum signal number
 *
 * @note only async-signal-safe functions should be used (man 7 signal-safety)
**/
/*@unused@*/
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
	union {	int d; } result;

	(void) write(STDERR_FILENO, intro0, (sizeof intro0) - 1u);
	(void) write(STDERR_FILENO, g_progname, strlen(g_progname));
	(void) write(STDERR_FILENO, intro1, (sizeof intro1) - 1u);
	(void) write(STDERR_FILENO, signame, strlen(signame));
	(void) write(STDERR_FILENO, intro2, (sizeof intro2) - 1u);

	// remove any incomplete file(s)
	if ( g_rm_on_sigint != NULL ){
		errwrite_action_start();
		result.d = unlink(g_rm_on_sigint);
		if ( (result.d != 0) && (errno == EACCES) ){	// /dev/null
			result.d = 0;
		}
		errwrite_action_end(result.d);
	}

	(void) write(STDERR_FILENO, outro, (sizeof outro) - 1u);
	_exit(signum);
}

/**@fn errwrite_action_start
 * @brief writes a '?' to stderr
**/
/*@unused@*/
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
/*@unused@*/
static void
errwrite_action_end(const int result)
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

//==========================================================================//

/// @see "system.h"
ALWAYS_INLINE void
timestamp_get(/*@out@*/ timestamp_p *const restrict out)
/*@globals	internalState@*/
{
	UNUSED const int rv = clock_gettime(CLOCK_MONOTONIC, out);
	assert(rv == 0);
	return;
}

/// @see "system.h"
INLINE PURE double
timestamp_diff(
	const timestamp_p *const restrict start,
	const timestamp_p *const restrict finish
)
/*@*/
{
	const double diff_sec  = (double) (
		finish->tv_sec - start->tv_sec
	);
	const double diff_nsec = (
		((double) (finish->tv_nsec - start->tv_nsec)) / 1000000000.0
	);
	return diff_sec + diff_nsec;
}

//==========================================================================//

/// @see "system.h"
ALWAYS_INLINE void
file_lock(FILE *const restrict filehandle)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		filehandle
@*/
{
	flockfile(filehandle);
	return;
}

/// @see "system.h"
ALWAYS_INLINE void
file_unlock(FILE *const restrict filehandle)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		filehandle
@*/
{
	funlockfile(filehandle);
	return;
}

//==========================================================================//

/// @see "system.h"
INLINE void
fdlimit_check(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	struct rlimit limit;
	union {	int d; } result;

	result.d = getrlimit((int) RLIMIT_NOFILE, &limit);
	if UNLIKELY ( result.d != 0 ){
		error_sys(errno, "getrlimit", NULL);
	}

	limit.rlim_cur = limit.rlim_max;

	result.d = setrlimit((int) RLIMIT_NOFILE, &limit);
	if UNLIKELY ( result.d != 0 ){
		error_sys(errno, "setrlimit", NULL);
	}

	return;
}

//==========================================================================//

/// @see "system.h"
INLINE uint
get_nprocessors_onln(void)
/*@globals	internalState*/
{
	return (uint) sysconf(_SC_NPROCESSORS_ONLN);
}

//==========================================================================//

/// @see "system.h"
/*@temp@*/
INLINE char *
strerror_ts(
	const int errnum, /*@out@*/ /*@returned@*/ char *const restrict buf,
	const size_t buflen
)
/*@modifies	*buf@*/
{
	union {	int d; } result;

	// XSI-compliant version returns an int
	result.d = strerror_r(errnum, buf, buflen);
	if ( result.d != 0 ){
		buf[0] = '\0';
	}
	return buf;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
