//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/encode.c                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>	// true
#include <stdlib.h>	// atoll, exit
#include <string.h>	// strtok

#include "../../bits.h"

#include "../debug.h"
#include "../formats.h"
#include "../help.h"
#include "../main.h"
#include "../opts.h"
#include "../optsget.h"

//////////////////////////////////////////////////////////////////////////////

#undef opt
static int opt_encode_rawpcm(uint, char *opt, enum OptMode)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*opt
@*/
;

static int opt_encode_help(uint, char *, enum OptMode)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
const struct OptDict encode_optdict[] = {
	{ "help"		, '?'	, opt_encode_help		},

	{ "single-threaded"	, 'S'	, opt_common_single_threaded	},
	{ "multi-threaded"	, 'M'	, opt_common_multi_threaded	},

	{ "delete-src"		, 'd'	, opt_common_delete_src		},
	{ "outfile"		, 'o'	, opt_common_outfile		},
	{ "quiet"		, 'q'	, opt_common_quiet		},
	{ "rawpcm"		, -1	, opt_encode_rawpcm		},
	{ "single-threaded"	, 'S'	, opt_common_single_threaded	},
	{ "threads"		, 't'	, opt_common_threads		},

	{ NULL , 0 , NULL }
};

//==========================================================================//

static struct {
	enum DecFormat		decfmt:8u;
	enum IntType		inttype:8u;
	u16			samplebits:8u;
	enum Endian		endian:8u;
	u16			nchan;
	u32			samplerate;
} f_rpstat;

//////////////////////////////////////////////////////////////////////////////

void
rawpcm_statcopy(struct FileStats *const restrict fstat)
/*@modifies	*fstat@*/
{
	fstat->decfmt		= f_rpstat.decfmt;
	fstat->inttype		= f_rpstat.inttype;
	fstat->samplebits	= f_rpstat.samplebits;
	fstat->endian		= f_rpstat.endian;
	fstat->nchan		= f_rpstat.nchan;
	fstat->samplerate	= f_rpstat.samplerate;
	return;
}

//==========================================================================//

// MAYBE add a g_rawpcm_stat thing instead; struct PcmStat
// --rawpcm=format,samplerate,nchan
static int
opt_encode_rawpcm(
	/*@unused@*/ uint optind, char *opt, /*@unused@*/ enum OptMode mode
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*opt
@*/
{
#ifndef S_SPLINT_S
	(void) optind;
	(void) mode;
#endif

	char *subopt;
	union {	longlong ll; } t;

	memset(&f_rpstat, 0x00, sizeof f_rpstat);

	(void) strtok(opt, "=");

	// format
	subopt = strtok(NULL, ",");
	if UNLIKELY ( subopt == NULL ){
		error_tta("%s: missing %s field", "--rawpcm", "format");
	}
	assert(subopt != NULL);

	if ( strcmp(subopt, "u8") == 0 ){
		f_rpstat.inttype    = INT_UNSIGNED;
		f_rpstat.samplebits = (u16) 8u;
	}
	else if ( strcmp(subopt, "i16le") == 0 ){
		f_rpstat.inttype    = INT_SIGNED;
		f_rpstat.samplebits = (u16) 16u;
		f_rpstat.endian     = xENDIAN_LITTLE;
	}
	else if ( strcmp(subopt, "i24le") == 0 ){
		f_rpstat.inttype    = INT_SIGNED;
		f_rpstat.samplebits = (u16) 24u;
		f_rpstat.endian     = xENDIAN_LITTLE;
	}
	else if UNLIKELY ( true ) {
		error_tta("%s: unsupported format: %s", "--rawpcm", subopt);
	}else{;}

	// samplerate
	subopt = strtok(NULL, ",");
	if UNLIKELY ( subopt == NULL ){
		error_tta("%s: missing %s field", "--rawpcm", "samplerate");
	}
	assert(subopt != NULL);

	t.ll = atoll(subopt);
	if UNLIKELY ( (t.ll <= 0) || (t.ll > (longlong) UINT32_MAX) ){
		error_tta("%s: %s out of range: %lld",
			"--rawpcm", "samplerate", t.ll
		);
	}
	f_rpstat.samplerate = (u32) t.ll;

	// nchan
	subopt = strtok(NULL, ",");
	if UNLIKELY ( subopt == NULL ){
		error_tta("%s: missing %s field", "--rawpcm", "nchan");
	}
	assert(subopt != NULL);

	t.ll = atoll(subopt);
	if UNLIKELY ( (t.ll <= 0) || (t.ll > (longlong) UINT16_MAX) ){
		error_tta("%s: %s out of range: %lld",
			"--rawpcm", "nchan", t.ll
		);
	}
	f_rpstat.nchan = (u16) t.ll;

	// decfmt
	f_rpstat.decfmt = DECFMT_RAWPCM;

	g_flag.rawpcm = true;
	return 0;
}

static int
opt_encode_help(
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
	errprint_help_mode_encode();
	exit(EXIT_SUCCESS);
}

// EOF ///////////////////////////////////////////////////////////////////////
