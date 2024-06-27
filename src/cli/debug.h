#ifndef TTA_DEBUG_H
#define TTA_DEBUG_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// debug.h                                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "formats.h"
#include "main.h"	// g_nwarnings

//////////////////////////////////////////////////////////////////////////////

#define T_RESET		"\033[0m"
#define T_B_DEFAULT 	"\033[0;1m"
//
#define T_DEFAULT 	"\033[;1m"
#define T_RED		"\033[31m"
#define T_YELLOW	"\033[33m"
#define T_PURPLE	"\033[35m"

enum Fatality {
	FATAL,
	NONFATAL
};

//////////////////////////////////////////////////////////////////////////////

extern NORETURN COLD void error_sys(
	int, const char *, /*@null@*/ const char *
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

extern COLD void error_sys_nf(int, const char *, /*@null@*/ const char *)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

/*@maynotreturn@*/
extern COLD void print_error_sys(
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
extern NORETURN COLD void error_tta(const char *, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

/*@printflike@*/
extern COLD void error_tta_nf(const char *, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

/*@printflike@*/
extern COLD void warning_tta(const char *, ...)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

extern COLD void error_filecheck(
	enum FileCheck, int, const struct FileStats *restrict,
	const char *restrict
)
/*@globals	fileSystem,
		g_nwarnings
@*/
/*@modifies	fileSystem,
		g_nwarnings
@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
