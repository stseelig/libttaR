//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/common.c                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <string.h>	// strtok

#include "../../bits.h"

#include "../debug.h"
#include "../formats.h"
#include "../main.h"	// PATH_DELIM
#include "../opts.h"
#include "../optsget.h"

//////////////////////////////////////////////////////////////////////////////

/*@checkmod@*/
struct FileStats g_fstat;

//////////////////////////////////////////////////////////////////////////////

int
opt_common_delete_src(
	/*@unused@*/ uint optind, /*@unused@*/ char *opt,
	/*@unused@*/ enum OptMode mode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.delete_src@*/
{
#ifndef S_SPLINT_S
	(void) optind;
	(void) opt;
	(void) mode;
#endif
	g_flag.delete_src = true;
	return 0;
}

// TODO shortopt
int
opt_common_outfile(uint optind, char *opt, enum OptMode mode)
/*@globals	fileSystem,
		internalState,
		g_flag
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.outfile,
		g_flag.outfile_is_dir,
		*opt
@*/
{
	int r;
	char *subopt;

	switch ( mode ){
	case OPTMODE_SHORT:
		if ( opt[1] == '\0' ){
			optsget_argcheck(optind, opt, 1u);
			subopt = g_argv[optind + 1u];
			r = -1;
		}
		else {	subopt = &opt[1];
			r = (int) strlen(subopt);
		}
		break;
	case OPTMODE_LONG:
		(void) strtok(opt, "=");
		subopt = strtok(NULL, "");
		if ( subopt == NULL ){
			error_tta("%s: missing %s field",
				"--format", "format"
			);
		}
		r = 0;
		break;
	}

	// check if directory
	if ( subopt[strlen(subopt) - 1u] == PATH_DELIM ){
		g_flag.outfile_is_dir = true;
	}
	else {	g_flag.outfile_is_dir = false;}

	g_flag.outfile = subopt;
	return r;
}

int
opt_common_quiet(
	/*@unused@*/ uint optind, /*@unused@*/ char *opt,
	/*@unused@*/ enum OptMode mode
)
/*@globals	g_flag@*/
/*@modifies	g_flag.quiet@*/
{
#ifndef S_SPLINT_S
	(void) optind;
	(void) opt;
	(void) mode;
#endif
	g_flag.quiet = true;
	return 0;
}

// EOF ///////////////////////////////////////////////////////////////////////
