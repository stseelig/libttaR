#ifndef TTA_MAIN_H
#define TTA_MAIN_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// main.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../bits.h"
#include "../libttaR.h"

//////////////////////////////////////////////////////////////////////////////

#define PATH_DELIM	'/'

//--------------------------------------------------------------------------//

enum ProgramMode {
	MODE_ENCODE,
	MODE_DECODE
};

enum ThreadMode {
	THREADMODE_UNSET,
	THREADMODE_SINGLE,
	THREADMODE_MULTI
};

//==========================================================================//

struct GlobalFlags {
	/*@dependent@*/ /*@null@*/
	char		*outfile;	// from argv
	bool		outfile_is_dir;
	bool		quiet;
	bool		delete_src;
	bool		rawpcm;
	enum ThreadMode	threadmode:8u;
	enum DecFormat	decfmt:8u;
};

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
extern const struct LibTTAr_VersionInfo ttaR_info;

//--------------------------------------------------------------------------//

#ifndef TTA_MAIN_C
/*@checkmod@*/ /*@temp@*/
extern const char *const g_progname;
#endif

/*@checkmod@*/
extern u8 g_nwarnings;

/*@checkmod@*/
extern struct GlobalFlags g_flag;

/*@checkmod@*/
extern uint g_nthreads;

/*@checkmod@*/ /*@dependent@*/ /*@null@*/
extern char *g_rm_on_sigint;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
