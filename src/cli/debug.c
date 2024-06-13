//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// debug.c                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>	// strerror_r

#include "../splint.h"

#include "debug.h"
#include "formats.h"	// FileCheck, FileStats, guid128_format
#include "main.h"

//////////////////////////////////////////////////////////////////////////////

NOINLINE COLD void print_error_tta(
	const enum Fatality fatality, const char *const, va_list args
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings,
		args
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

COLD NORETURN void
error_sys(
	int errnum, const char *const name,
	/*@null@*/ const char *const extra
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
{
	print_error_sys(errnum, name, extra, FATAL);
	UNREACHABLE;
}

COLD void
error_sys_nf(
	int errnum, const char *const name,
	/*@null@*/ const char *const extra
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
{
	print_error_sys(errnum, name, extra, NONFATAL);
}

COLD void
print_error_sys(
	int errnum, const char *const name,
	/*@null@*/ const char *const extra, const enum Fatality fatality
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
{
	char buf[128u];
	union {	int d; } t;

	flockfile(stdout);
	flockfile(stderr);
	//
	(void) fprintf(stderr, T_B_DEFAULT "%s: ", g_argv[0]);
	(void) fputs(T_B_RED "error:" T_B_DEFAULT " ", stderr);
	(void) fprintf(stderr, "%s: (%d) ", name, errnum);
	(void) fputs(strerror_r(errnum, buf, sizeof buf), stderr);
	if ( extra != NULL ){
		(void) fprintf(stderr, ": %s", extra);
	}
	(void) fputs(" " T_B_RED "!" T_RESET "\n", stderr);
	//
	funlockfile(stdout);
	funlockfile(stderr);

	t.d = inc_nwarnings();
	if ( fatality == FATAL ){
		exit(t.d);
	}
	else {	return; }
}

//--------------------------------------------------------------------------//

COLD NORETURN void error_tta(const char *const format, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
{
	va_list args;
	va_start(args, format);
	print_error_tta(FATAL, format, args);
	UNREACHABLE;
}

COLD void error_tta_nf(const char *const format, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
{
	va_list args;
	va_start(args, format);
	print_error_tta(NONFATAL, format, args);
	va_end(args);
	return;
}

NOINLINE COLD void
print_error_tta(
	const enum Fatality fatality, const char *const format, va_list args
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings,
		args
@*/
{
	union {	int d; } t;

	flockfile(stdout);
	flockfile(stderr);
	//
	(void) fprintf(stderr, T_B_DEFAULT "%s: ", g_argv[0]);
	(void) fputs(T_B_RED "error:" T_B_DEFAULT " ", stderr);
	(void) vfprintf(stderr, format, args);
	(void) fputs(" " T_B_RED "!" T_RESET "\n", stderr);
	//
	funlockfile(stdout);
	funlockfile(stderr);

	t.d = inc_nwarnings();
	if ( fatality == FATAL ){
		exit(t.d);
	}
	else {	return;	}
}

COLD void
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

	flockfile(stdout);
	flockfile(stderr);
	//
	(void) fprintf(stderr, T_B_DEFAULT "%s: ", g_argv[0]);
	(void) fputs(T_B_YELLOW "warning:" T_B_DEFAULT " ", stderr);
	(void) vfprintf(stderr, format, args);
	(void) fputs(" " T_B_YELLOW "!" T_RESET "\n", stderr);
	//
	funlockfile(stdout);
	funlockfile(stderr);

	va_end(args);

	(void) inc_nwarnings();
	return;
}

//--------------------------------------------------------------------------//

COLD void
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
	char guidbuf[GUID128_BUFLEN];

	switch ( fc ){
	case FILECHECK_OK:
		error_tta_nf("%s: OK", filename);
		break;
	case FILECHECK_READ_ERROR:
		error_sys_nf(errnum, "fread", filename);
		break;
	case FILECHECK_SEEK_ERROR:
		error_sys_nf(errnum, "fseeko", filename);
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
				guid128_format(
					guidbuf, sizeof guidbuf,
					&fstat->wavsubformat
				)
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

// EOF ///////////////////////////////////////////////////////////////////////
