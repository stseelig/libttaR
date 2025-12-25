#ifndef H_TTA_DEBUG_H
#define H_TTA_DEBUG_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// debug.h                                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "./common.h"
#include "./formats.h"
#include "./main.h"

/* //////////////////////////////////////////////////////////////////////// */

#define T_RESET		"\033[0m"
#define T_B_DEFAULT 	"\033[0;1m"

#define T_DEFAULT 	"\033[;1m"
#define T_RED		"\033[31m"
#define T_YELLOW	"\033[33m"
#define T_PURPLE	"\033[35m"

/* ======================================================================== */

enum Fatality {
	FATAL,
	NONFATAL
};

/* //////////////////////////////////////////////////////////////////////// */

NORETURN COLD
BUILD_EXTERN NOINLINE void error_sys(
	int, const char *, /*@null@*/ const char *
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

COLD
BUILD_EXTERN NOINLINE void error_sys_nf(
	int, const char *, /*@null@*/ const char *
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

/*@maynotreturn@*/
COLD
BUILD_EXTERN void print_error_sys(
	int, const char *, /*@null@*/ const char *, enum Fatality
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

/*@printflike@*/
NORETURN COLD
BUILD_EXTERN NOINLINE void error_tta(const char *, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

/*@printflike@*/
COLD
BUILD_EXTERN NOINLINE void error_tta_nf(const char *, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

/*@printflike@*/
COLD
BUILD_EXTERN NOINLINE void warning_tta(const char *, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

COLD
BUILD_EXTERN NOINLINE void error_filecheck(
	enum FileCheck, int, const struct FileStats *RESTRICT,
	const char *RESTRICT
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_DEBUG_H */
