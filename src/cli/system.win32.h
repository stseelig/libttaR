#ifndef TTA_SYSTEM_WIN32_H
#define TTA_SYSTEM_WIN32_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// system.win32.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>

#include <io.h>
#include <windows.h>
//#include <profileapi.h>
//#include <sysinfoapi.h>

#include "debug.h"
#include "main.h"	// g_progname

//////////////////////////////////////////////////////////////////////////////

#define PATH_DELIM	'\\'

typedef LARGE_INTEGER	timestamp_p;

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
	signal(SIGABRT, sighand_cleanup_exit);
	signal(SIGINT , sighand_cleanup_exit);
	signal(SIGTERM, sighand_cleanup_exit);
	return;
}

//--------------------------------------------------------------------------//

/// @see "system.posix.h"
/*@unused@*/
static NORETURN COLD void
sighand_cleanup_exit(const int signum)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char intro0[]  = "\n" T_B_DEFAULT;
	const char intro1[]  = ": " T_PURPLE;
	const char signame[] = "Signal";
	const char intro2[]  = T_DEFAULT " ";
	const char outro[]   = T_PURPLE "!" T_RESET "\n";
	//
	union {	int d; } t;

	(void) _write(STD_ERROR_HANDLE, intro0, (sizeof intro0) - 1u);
	(void) _write(STD_ERROR_HANDLE, g_progname, strlen(g_progname));
	(void) _write(STD_ERROR_HANDLE, intro1, (sizeof intro1) - 1u);
	(void) _write(STD_ERROR_HANDLE, signame, (sizeof signame) - 1u);
	(void) _write(STD_ERROR_HANDLE, intro2, (sizeof intro2) - 1u);

	// remove any incomplete file(s)
	if ( g_rm_on_sigint != NULL ){
		errwrite_action_start();
		t.d = _unlink(g_rm_on_sigint);
		if ( (t.d != 0) && (errno == EACCES) ){	// /dev/null
			t.d = 0;
		}
		errwrite_action_end(t.d);
	}

	(void) _write(STD_ERROR_HANDLE, outro, (sizeof outro) - 1u);
	_exit(signum);
}

/// @see "system.posix.h"
/*@unused@*/
static void
errwrite_action_start(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char act_start[] = "?";

	(void) _write(STD_ERROR_HANDLE, act_start, (sizeof act_start) - 1u);
	return;
}

/// @see "system.posix.h"
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
	(void) _write(STD_ERROR_HANDLE, str, size);
	return;
}

//==========================================================================//

/// @see "system.h"
ALWAYS_INLINE void
timestamp_get(/*@out@*/ timestamp_p *const restrict out)
/*@globals	internalState@*/
{
	UNUSED const BOOL rv = QueryPerformanceCounter(out);
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
	LARGE_INTEGER freq_li;
	double freq_g, start_sec, finish_sec;

	QueryPerformanceFrequency(&freq_li);
	freq_g     =  (double) freq_li.QuadPart;
	start_sec  = ((double)  start->QuadPart) / freq_g;
	finish_sec = ((double) finish->QuadPart) / freq_g;

	return finish_sec - start_sec;
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
	_lock_file(filehandle);
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
	_unlock_file(filehandle);
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
	union {	int d; } t;

	do {	t.d = _setmaxstdio(2 * _getmaxstdio());
	} while ( t.d > 0 );
	return;
}

//==========================================================================//

/// @see "system.h"
INLINE uint
get_nprocessors_onln(void)
/*@globals	internalState*/
{
	SYSTEM_INFO info;

	GetSystemInfo(&info);
	return (uint) info.dwNumberOfProcessors;
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
	union {	errno_t e; } t;

	t.e = strerror_s(buf, buflen, errnum);
	if ( t.e != 0 ){
		buf[0] = '\0';
	}
	return buf;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
