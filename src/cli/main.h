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

//////////////////////////////////////////////////////////////////////////////

#define SAMPLEBUF_LEN_DEFAULT		((size_t) BUFSIZ)

// multi-threaded ver. can deadlock or abort if (framequeue_len <= nthreads)
#define FRAMEQUEUE_LEN(nthreads)	((uint) (2u*(nthreads)))

//--------------------------------------------------------------------------//

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
	char		*outfile;	// from g_argv
	bool		outfile_is_dir;
	bool		quiet;
	bool		delete_src;
	bool		rawpcm;
	enum ThreadMode	threadmode:8u;
	enum DecFormat	decfmt:8u;
};

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
extern const uint ttaR_num_version;
/*@unchecked@*/
extern const uint ttaR_num_version_major;
/*@unchecked@*/
extern const uint ttaR_num_version_minor;
/*@unchecked@*/
extern const uint ttaR_num_version_revis;

/*@unchecked@*/
extern const char ttaR_str_version_extra[];
/*@unchecked@*/
extern const char ttaR_str_version_date[];

/*@unchecked@*/
extern const char ttaR_str_copyright[];
/*@unchecked@*/
extern const char ttaR_str_license[];


//--------------------------------------------------------------------------//

#ifndef TTA_MAIN_C
/*@checkmod@*/
extern const uint g_argc;
#endif

/*@checkmod@*/ /*@dependent@*/
extern char **g_argv;

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
