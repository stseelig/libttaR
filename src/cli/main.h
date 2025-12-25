#ifndef H_TTA_MAIN_H
#define H_TTA_MAIN_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// main.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdbool.h>
#include <stdint.h>

#include "../libttaR.h"

#include "./common.h"

/* //////////////////////////////////////////////////////////////////////// */

enum ProgramMode {
	MODE_ENCODE,
	MODE_DECODE
};

enum ThreadMode {
	THREADMODE_UNSET,
	THREADMODE_SINGLE,
	THREADMODE_MULTI
};

/* ======================================================================== */

struct GlobalFlags {
	/*@dependent@*/ /*@null@*/
	char		*outfile;		/* from argv */
	bool		 outfile_is_dir;
	bool		 quiet;
	bool		 delete_src;
	bool		 rawpcm;
	enum ThreadMode	 threadmode:8u;
	enum DecFormat	 decfmt:8u;
};

/* //////////////////////////////////////////////////////////////////////// */

/*@-redef@*/

/*@unchecked@*/
BUILD_EXTERN const struct LibTTAr_VersionInfo ttaR_info;

/*@checkmod@*/ /*@temp@*/
BUILD_EXTERN const char *g_progname;

/*@checkmod@*/
BUILD_EXTERN uint8_t g_nwarnings;

/*@checkmod@*/
BUILD_EXTERN struct GlobalFlags g_flag;

/*@checkmod@*/
BUILD_EXTERN unsigned int g_nthreads;

/*@checkmod@*/ /*@dependent@*/ /*@null@*/
BUILD_EXTERN char *g_rm_on_sigint;

/*@=redef@*/

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_MAIN_H */
