//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// debug.c                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// errors are macro'd in header to fatal (normal) / non-fatal (_nf suffix)  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef S_SPLINT_S
#include "../splint.h"
#endif

/* ------------------------------------------------------------------------ */

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

#include "debug.h"
#include "formats.h"	// FileCheck, FileStats, guid128_format
#include "main.h"
#include "system.h"

//////////////////////////////////////////////////////////////////////////////

static NOINLINE COLD void print_error_tta(
	enum Fatality fatality, const char *, va_list args
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

/**@fn int_nwarnings
 * @brief increment the number of error/warnings variable
 *
 * @return value of g_nwarnings
 *
 * @note g_nwarnings is the return value of the program
**/
static int
inc_nwarnings(void)
/*@globals	g_nwarnings@*/
/*@modifies	g_nwarnings@*/
{
	return (int) (g_nwarnings += (u8) (g_nwarnings < UINT8_MAX));
}

//==========================================================================//

/**@fn error_sys
 * @brief print a fatal system error
 *
 * @param errnum error number
 * @param name[in] function name
 * @param extra[in] any extra info, probably a filename
**/
NORETURN COLD void
error_sys(
	const int errnum, const char *const name,
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

/**@fn error_sys_nf
 * @brief print a non-fatal system error
 *
 * @param errnum error number
 * @param name[in] function name
 * @param extra[in] any extra info, probably a filename
**/
COLD void
error_sys_nf(
	const int errnum, const char *const name,
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

/**@fn print_error_sys
 * @brief print a system error
 *
 * @param errnum error number
 * @param name[in] function name
 * @param extra[in] any extra info, probably a filename
 * @param fatality whether the error is fatal or non-fatal
**/
/*@maynotreturn@*/
COLD void
print_error_sys(
	const int errnum, const char *const name,
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
	int  nwarnings;

	file_lock(stdout);
	file_lock(stderr);
	//
	(void) fprintf(stderr, T_B_DEFAULT "%s: ", g_progname);
	(void) fputs(T_RED "error:" T_DEFAULT " ", stderr);
	(void) fprintf(stderr, "%s: (%d) ", name, errnum);
	(void) fputs(strerror_ts(errnum, buf, sizeof buf), stderr);
	if ( extra != NULL ){
		(void) fprintf(stderr, ": %s", extra);
	}
	(void) fputs(" " T_RED "!" T_RESET "\n", stderr);
	//
	file_unlock(stdout);
	file_unlock(stderr);

	nwarnings = inc_nwarnings();
	if ( fatality == FATAL ){
		exit(nwarnings);
	}
	else {	return; }
}

//--------------------------------------------------------------------------//

/**@fn error_tta
 * @brief print a fatal program error
 *
 * @param format[in] formatted error string
 * @param ...[in] args for 'format'
**/
/*@printflike@*/
NORETURN COLD void
error_tta(const char *const format, ...)
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

/**@fn error_tta_nf
 * @brief print a non-fatal program error
 *
 * @param format[in] formatted error string
 * @param ...[in] args for 'format'
**/
/*@printflike@*/
COLD void
error_tta_nf(const char *const format, ...)
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

/**@fn print_error_tta
 * @brief print a non-fatal program error
 *
 * @param fatality whether the error is fatal or non-fatal
 * @param format[in] formatted error string
 * @param args[in] args for 'format'
**/
/*@maynotreturn@*/
static NOINLINE COLD void
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
	int nwarnings;

	file_lock(stdout);
	file_lock(stderr);
	//
	(void) fprintf(stderr, T_B_DEFAULT "%s: ", g_progname);
	(void) fputs(T_RED "error:" T_DEFAULT " ", stderr);
	(void) vfprintf(stderr, format, args);
	(void) fputs(" " T_RED "!" T_RESET "\n", stderr);
	//
	file_unlock(stdout);
	file_unlock(stderr);

	nwarnings = inc_nwarnings();
	if ( fatality == FATAL ){
		exit(nwarnings);
	}
	else {	return;	}
}

/**@fn print_error_tta
 * @brief print a non-fatal program warning
 *
 * @param format[in] formatted error string
 * @param ...[in] args for 'format'
**/
/*@printflike@*/
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

	file_lock(stdout);
	file_lock(stderr);
	//
	(void) fprintf(stderr, T_B_DEFAULT "%s: ", g_progname);
	(void) fputs(T_YELLOW "warning:" T_DEFAULT " ", stderr);
	(void) vfprintf(stderr, format, args);
	(void) fputs(" " T_YELLOW "!" T_RESET "\n", stderr);
	//
	file_unlock(stdout);
	file_unlock(stderr);

	va_end(args);

	(void) inc_nwarnings();
	return;
}

//--------------------------------------------------------------------------//

/**@fn error_filecheck
 * @brief print a non-fatal filecheck error
 *
 * @param fc the error type
 * @param errnum error number
 * @param fstat[in] the bloated file stats struct
 * @param filename[in] the name of the source file
 *
 * @note These eventually lead to a fatal error. I just wanted to print them
 *   all out before exiting.
**/
COLD void
error_filecheck(
	const enum FileCheck fc, const int errnum,
	const struct FileStats *const restrict fstat,
	const char *const restrict filename
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
			"%"PRIu16"-ch, %"PRIu16"-bits, %"PRIu32"-Hz",
			filename, fstat->nchan, fstat->samplebits,
			fstat->samplerate
		);
		break;
	}
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
