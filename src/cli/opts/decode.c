//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/decode.c                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdlib.h>	// exit
#include <string.h>	// strtok

#include "../../bits.h"
#include "../../splint.h"

#include "../debug.h"
#include "../formats.h"
#include "../help.h"
#include "../main.h"
#include "../opts.h"
#include "../optsget.h"

#include "common.h"

//////////////////////////////////////////////////////////////////////////////

#undef argv
static int opt_decode_format(
	uint, uint, uint, char *const *argv, enum OptMode
)
/*@globals	fileSystem,
		internalState,
		g_flag
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.decfmt,
		**argv
@*/
;

static int
opt_decode_help(
	uint, uint, uint, char *const *, enum OptMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

//////////////////////////////////////////////////////////////////////////////

#define DECODE_OPTDICT_NMEMB	((uint) 8u)

/*@observer@*/ /*@unchecked@*/
static const char *decode_optdict_longopt[DECODE_OPTDICT_NMEMB] = {
	"single-threaded",
	"multi-threaded",
	"delete-src",
	"format",
	"outfile",
	"quiet",
	"threads",
	"help"
};

/*@unchecked@*/
static const int decode_optdict_shortopt[DECODE_OPTDICT_NMEMB] = {
	'S',	// single-threaded
	'M',	// multi-threaded
	'd',	// delete-src
	'f',	// format
	'o',	// outfile
	'q',	// quiet
	't',	// threads
	'?'	// help
};

/*@unchecked@*/
static int (*const decode_optdict_fp[DECODE_OPTDICT_NMEMB])
(uint, uint, uint, char *const *, enum OptMode) = {
	opt_common_single_threaded,
	opt_common_multi_threaded,
	opt_common_delete_src,
	opt_decode_format,
	opt_common_outfile,
	opt_common_quiet,
	opt_common_threads,
	opt_decode_help,
};

/*@unchecked@*/
const struct OptDict decode_optdict = {
	.nmemb    = DECODE_OPTDICT_NMEMB,
	.longopt  = decode_optdict_longopt,
	.shortopt = decode_optdict_shortopt,
	.fn       = decode_optdict_fp
};

//////////////////////////////////////////////////////////////////////////////

/**@fn opt_decode_format
 * @brief sets the destination file format
 *
 * @param optind0 the index of  'argv'
 * @param optind1 the index of *'argv'
 * @param argc the argument count from main()
 * @param argv[in out] the argument vector from main()
 * @param mode short or long
 *
 * @return number of args used (long), or number of char's read (short)
**/
static int
opt_decode_format(
	const uint optind0, const uint optind1, const uint argc,
	char *const *const argv, const enum OptMode mode
)
/*@globals	fileSystem,
		internalState,
		g_flag
@*/
/*@modifies	fileSystem,
		internalState,
		g_flag.decfmt,
		**argv
@*/
{
	int r;
	/*@observer@*/
	const char *const decfmt_name[] = DECFMT_NAME_ARRAY;
	char *const opt = &argv[optind0][optind1];
	char *subopt;
	uint i;

	switch ( mode ){
	case OPTMODE_SHORT:
		if ( opt[1u] == '\0' ){
			optsget_argcheck(optind0, argc, 1u, opt);
			subopt = argv[optind0 + 1u];
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
			error_tta("%s: missing argument", "--format");
		}
		r = 0;
		break;
	}
	assert(subopt != NULL);

	for ( i = 0; i < NUM_DECFMT; ++i ){
		if ( strcmp(subopt, decfmt_name[i]) == 0 ){
			g_flag.decfmt = (enum DecFormat) i;
			break;
		}
		else if UNLIKELY ( i == NUM_DECFMT - 1u ){
			error_tta("%s: bad argument: %s",
				mode == OPTMODE_SHORT ? "-f" : "--format",
				subopt
			);
		} else{;}
	}

	return r;
}

/**@fn opt_decode_help
 * @brief print the mode_decode help to stderr and exit
 *
 * @param optind0 unused
 * @param optind1 unused
 * @param argc unused
 * @param argv unused
 * @param mode unused
 *
 * @return does not return
**/
NORETURN COLD int
opt_decode_help(
	UNUSED const uint optind0, UNUSED const uint optind1,
	UNUSED const uint argc, UNUSED char *const *const argv,
	UNUSED const enum OptMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	errprint_help_mode_decode();
	exit(EXIT_SUCCESS);
}

// EOF ///////////////////////////////////////////////////////////////////////
