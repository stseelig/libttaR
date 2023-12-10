//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// debug.c                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// errors are macro'd in header to fatal (normal) / non-fatal (_nf suffix)  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <errno.h>
#include <inttypes.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/resource.h>

#include "../splint.h"

#include "debug.h"
#include "formats.h"	// FileCheck, FileStats, guid128_format
#include "main.h"

//////////////////////////////////////////////////////////////////////////////

static int inc_nwarnings(void)
/*@globals	g_nwarnings@*/
/*@modifies	g_nwarnings@*/
;

static bool try_fdlimit(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

static void fdlimit_check(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

//////////////////////////////////////////////////////////////////////////////

static int inc_nwarnings(void)
/*@globals	g_nwarnings@*/
/*@modifies	g_nwarnings@*/
{
	if ( g_nwarnings < UINT8_MAX ){
		++g_nwarnings;
	}
	return (int) g_nwarnings;
}

//==========================================================================//

void
print_error_sys(
	enum Fatality fatality, int errnum, const char *const name,
	/*@null@*/ const char *const msg0, /*@null@*/ const char *const msg1
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
{
	union {
		int d;
	} t;

	(void) fprintf(stderr, T_B_DEFAULT "%s: ", g_argv[0]);
	(void) fprintf(stderr, T_B_RED "error:" T_B_DEFAULT " ");
	(void) fprintf(stderr, "%s: (%d)", name, errnum);
	if ( msg0 != NULL ){
		(void) fprintf(stderr, " %s", msg0);
	}
	if ( msg1 != NULL ){
		(void) fprintf(stderr, ": %s", msg1);
	}
	(void) fprintf(stderr, " " T_B_RED "!" T_RESET "\n");

	t.d = inc_nwarnings();
	if ( fatality == FATAL ){
		exit(t.d);
	}
	else {	return; }
}

void
print_error_tta(enum Fatality fatality, const char *const format, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
{
	va_list args;
	union {
		int d;
	} t;

	va_start(args, format);

	(void) fprintf(stderr, T_B_DEFAULT "%s: ", g_argv[0]);
	(void) fprintf(stderr, T_B_RED "error:" T_B_DEFAULT " ");
	(void) vfprintf(stderr, format, args);
	(void) fprintf(stderr, " " T_B_RED "!" T_RESET "\n");

	t.d = inc_nwarnings();
	if ( fatality == FATAL ){
		exit(t.d);
	}
	else {	return; }
}

void
warning_tta(const char *const format, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
{
	va_list args;

	va_start(args, format);

	(void) fprintf(stderr, T_B_DEFAULT "%s: ", g_argv[0]);
	(void) fprintf(stderr, T_B_YELLOW "warning:" T_B_DEFAULT " ");
	(void) vfprintf(stderr, format, args);
	(void) fprintf(stderr, " " T_B_YELLOW "!" T_RESET "\n");

	va_end(args);

	(void) inc_nwarnings();
	return;
}

//--------------------------------------------------------------------------//

void
error_filecheck(
	enum FileCheck fc, const struct FileStats *const restrict fstat,
	const char *const restrict filename, int errnum
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
{
	char guidbuf[GUID128_BUFSIZE];

	switch ( fc ){
	case FILECHECK_OK:
		error_tta_nf("%s: OK", filename);
		break;
	case FILECHECK_READ_ERROR:
		error_sys_nf(errnum, "fread", strerror(errnum), filename);
		break;
	case FILECHECK_SEEK_ERROR:
		error_sys_nf(errnum, "fseeko", strerror(errnum), filename);
		break;
	case FILECHECK_MISMATCH:
		error_tta_nf("%s: unsupported filetype", filename);
		break;
	case FILECHECK_MALFORMED:
		error_tta_nf("%s: malformed file", filename);
		break;
	case FILECHECK_CORRUPTED:
		error_tta_nf("%s: corrupted file", filename);
		break;
	case FILECHECK_UNSUPPORTED_DATATYPE:
		if ( fstat->wavformat != WAVE_FMT_EXTENSIBLE ){
			error_tta_nf("%s: unsupported datatype: 0x%04"PRIX16,
				filename, fstat->wavformat
			);
		}
		else {	error_tta_nf("%s: unsupported datatype: %s",
				filename,
				guid128_format(guidbuf, &fstat->wavsubformat)
			);
		}
		break;
	case FILECHECK_UNSUPPORTED_RESOLUTION:
		error_tta_nf("%s: unsupported resolution: "
			"%"PRIu16"-ch, %u-bits, %"PRIu32"-Hz",
			filename, fstat->nchan, fstat->samplebits,
			fstat->samplerate
		);
		break;
	}
	return;
}

//==========================================================================//

/*@dependent@*/ /*@null@*/
FILE
*fopen_check(const char *pathname, const char *mode, enum Fatality fatality)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	FILE *r;

try_again:
	r = fopen(pathname, mode);
	if ( r == NULL ){
		if ( try_fdlimit() ){ goto try_again; }
		else {	print_error_sys(
				fatality, errno, "fopen", strerror(errno),
				pathname
			);
		}
	}
	return r;
}

//--------------------------------------------------------------------------//

// true:  continue
// false: fallthrough
static bool
try_fdlimit(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	static bool tried;

	if ( ! tried ){
		fdlimit_check();
		tried = true;
		return true;
	}
	return false;
}

// attempt to increase the open-file limit
static void
fdlimit_check(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	struct rlimit limit;
	union {
		int d;
	} t;

	t.d = getrlimit((int) RLIMIT_NOFILE, &limit);
	if ( t.d != 0 ){
		error_sys(errno, "getrlimit", strerror(errno), NULL);
	}

	limit.rlim_cur = limit.rlim_max;

	t.d = setrlimit((int) RLIMIT_NOFILE, &limit);
	if ( t.d != 0 ){
		error_sys(errno, "setrlimit", strerror(errno), NULL);
	}

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
