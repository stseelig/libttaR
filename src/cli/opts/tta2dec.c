//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/tta2dec.c                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdlib.h>	// exit
#include <string.h>	// strtok

#include "../../bits.h"

#include "../debug.h"
#include "../help.h"
#include "../main.h"
#include "../opts.h"
#include "../optsget.h"

//////////////////////////////////////////////////////////////////////////////

#undef opt
static int opt_tta2dec_format(uint, char *opt, enum OptMode)
/*@globals	fileSystem,
		internalState,
		g_flag
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.decfmt,
		opt
@*/
;

static int
opt_tta2dec_help(uint, char *,enum OptMode)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
const struct OptDict tta2dec_optdict[] = {
	{ "help"		, '?'	, opt_tta2dec_help		},

	{ "single-threaded"	, 'S'	, opt_common_single_threaded	},
	{ "multi-threaded"	, 'M'	, opt_common_multi_threaded	},

	{ "delete-src"		, 'd'	, opt_common_delete_src		},
	{ "format"		, 'f'	, opt_tta2dec_format		},
	{ "outfile"		, 'o'	, opt_common_outfile		},
	{ "quiet"		, 'q'	, opt_common_quiet		},
	{ "single-threaded"	, 'S'	, opt_common_single_threaded	},
	{ "threads"		, 't'	, opt_common_threads		},

	{ NULL , 0 , NULL }
};

//////////////////////////////////////////////////////////////////////////////

static int
opt_tta2dec_format(uint optind, char *opt, enum OptMode mode)
/*@globals	fileSystem,
		internalState,
		g_flag
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.decfmt,
		opt
@*/
{
	int r;
	const char *decfmt_name[] = {"raw", "wav", "w64"};
	#define NUM_DECFMT ((uint) ((sizeof decfmt_name) / sizeof( char *)))
	char *subopt;
	uint i;

	switch ( mode ){
	case OPTMODE_SHORT:
		if ( opt[1u] == '\0' ){
			optsget_argcheck(optind, opt, 1u);
			subopt = g_argv[optind + 1u];
			r = -1;
		}
		else {	subopt = &opt[1u];
			r = (int) strlen(subopt);
		}
		break;
	case OPTMODE_LONG:
		(void) strtok(opt, "=");
		subopt = strtok(NULL, "");
		if UNLIKELY ( subopt == NULL ){
			error_tta("%s: missing %s field",
				"--format", "format"
			);
		}
		r = 0;
		break;
	}

	for ( i = 0; i < NUM_DECFMT; ++i ){
		if ( strcmp(subopt, decfmt_name[i]) == 0 ){
			g_flag.decfmt = (enum DecFormat) i;
			break;
		}
		else if UNLIKELY ( i == NUM_DECFMT - 1u ){
			error_tta("%s: bad %s: %s",
				mode == OPTMODE_SHORT ? "-f" : "--format",
				"format", subopt
			);
		} else{;}
	}

	return r;
}

static int
opt_tta2dec_help(
	/*@unused@*/ uint optind, /*@unused@*/ char *opt,
	/*@unused@*/ enum OptMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
#ifndef S_SPLINT_S
	(void) optind;
	(void) opt;
	(void) mode;
#endif
	errprint_help_mode_decode();
	exit(EXIT_SUCCESS);
}

// EOF ///////////////////////////////////////////////////////////////////////
