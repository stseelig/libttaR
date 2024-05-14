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

#define G_SAMPLEBUF_LEN_DEFAULT	((size_t) 2u*BUFSIZ)

//--------------------------------------------------------------------------//

#define PATH_DELIM	'/'

enum ProgramMode {
	MODE_ENCODE,
	MODE_DECODE
};

enum ThreadMode {
	THREADMODE_UNSET,
	THREADMODE_SINGLE,
	THREADMODE_MULTI
};

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

#ifndef TTA_MAIN_C
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

/*@checkmod@*/
extern const uint g_argc;

/*@checkmod@*/ /*@dependent@*/
extern char **g_argv;

/*@checkmod@*/
extern u8 g_nwarnings;

/*@checkmod@*/
extern struct GlobalFlags g_flag;

/*@checkmod@*/
extern size_t  g_samplebuf_len;

/*@checkmod@*/
extern int g_nthreads;

/*@checkmod@*/ /*@dependent@*/ /*@null@*/
extern char *g_rm_on_sigint;
#endif

// EOF ///////////////////////////////////////////////////////////////////////
#endif
