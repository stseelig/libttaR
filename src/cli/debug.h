#ifndef TTA_DEBUG_H
#define TTA_DEBUG_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// debug.h                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "formats.h"
#include "main.h"	// g_argv

//////////////////////////////////////////////////////////////////////////////

// TODO global variable instead
//#define PROGRAM		"tta2enc"

#define T_RESET		"\033[0m"
#define T_B_DEFAULT 	"\033[0;1m"
#define T_B_RED		"\033[1;31m"
#define T_B_YELLOW	"\033[1;33m"
#define T_PURPLE	"\033[35m"

enum Fatality {
	FATAL,
	NONFATAL
};

//////////////////////////////////////////////////////////////////////////////

extern void print_error_sys(
	enum Fatality fatality, int errnum, const char *const name,
	/*@null@*/ const char *const msg0, /*@null@*/ const char *const msg1
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

extern void error_sys(
	int, const char *const, /*@null@*/ const char *const,
	/*@null@*/ const char *const
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

extern void error_sys_nf(
	int, const char *const, /*@null@*/ const char *const,
	/*@null@*/ const char *const
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

extern void print_error_tta(
	enum Fatality fatality, const char *const format, ...
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

extern void error_tta(const char *const format, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

extern void error_tta_nf(const char *const format, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

extern void warning_tta(const char *const, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

extern void error_filecheck(
	enum FileCheck, const struct FileStats *const restrict,
	const char *const restrict, int
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

#define error_sys(errnum, name, msg0, msg1) \
	print_error_sys(FATAL, errnum, name, msg0, msg1)

#define error_sys_nf(errnum, name, msg0, msg1) \
	print_error_sys(NONFATAL, errnum, name, msg0, msg1)

#define error_tta(...) \
	print_error_tta(FATAL, __VA_ARGS__)

#define error_tta_nf(...) \
	print_error_tta(NONFATAL, __VA_ARGS__)

//--------------------------------------------------------------------------//

/*@dependent@*/ /*@null@*/
extern FILE *fopen_check(
	const char *pathname, const char *mode, enum Fatality
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
