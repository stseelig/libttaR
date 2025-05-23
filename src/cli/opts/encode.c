//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// opts/encode.c                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef S_SPLINT_S
#include "../../splint.h"
#endif

/* ------------------------------------------------------------------------ */

#include <assert.h>
#include <stdbool.h>	// true
#include <stdlib.h>	// atoll, exit
#include <string.h>	// strtok

#include "../../bits.h"

#include "../debug.h"
#include "../formats.h"
#include "../help.h"
#include "../main.h"

#include "common.h"
#include "optsget.h"

//////////////////////////////////////////////////////////////////////////////

#undef argv
static int opt_encode_rawpcm(
	uint, uint, uint, char *const *argv, enum OptMode
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
;

static int opt_encode_help(
	uint, uint, uint, char *const *, enum OptMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

//////////////////////////////////////////////////////////////////////////////

#define xENCODE_OPTDICT_NMEMB	((uint) 8u)

/**@var encode_optdict_longopt
 * @brief array of longopts
**/
/*@observer@*/ /*@unchecked@*/
static const char *encode_optdict_longopt[xENCODE_OPTDICT_NMEMB] = {
	"single-threaded",
	"multi-threaded",
	"delete-src",
	"outfile",
	"quiet",
	"rawpcm",
	"threads",
	"help"
};

/**@var encode_optdict_shortopt
 * @brief array of shortopts
**/
/*@unchecked@*/
static const int encode_optdict_shortopt[xENCODE_OPTDICT_NMEMB] = {
	'S',	// single-threaded
	'M',	// multi-threaded
	'd',	// delete-src
	'o',	// outfile
	'q',	// quiet
	-1,	// rawpcm
	't',	// threads
	'?'	// help
};

/**@var encode_optdict_fn
 * @brief array of option function pointers
**/
/*@unchecked@*/
static int (*const encode_optdict_fn[xENCODE_OPTDICT_NMEMB])
(uint, uint, uint, char *const *, enum OptMode) = {
	opt_common_single_threaded,
	opt_common_multi_threaded,
	opt_common_delete_src,
	opt_common_outfile,
	opt_common_quiet,
	opt_encode_rawpcm,
	opt_common_threads,
	opt_encode_help
};

/**@struct encode_optdict
 * @brief option dictionary for optargs_process
**/
/*@unchecked@*/
const struct OptDict encode_optdict = {
	.nmemb    = xENCODE_OPTDICT_NMEMB,
	.longopt  = encode_optdict_longopt,
	.shortopt = encode_optdict_shortopt,
	.fn       = encode_optdict_fn
};

//==========================================================================//

/**@struct f_rpstat
 * @brief the compact file stat struct for encoding raw PCM
**/
static struct {
	enum DecFormat		decfmt:8u;
	enum IntType		inttype:8u;
	u16			samplebits:8u;
	enum Endian		endian:8u;
	u16			nchan;
	u32			samplerate;
} f_rpstat;

//////////////////////////////////////////////////////////////////////////////

/**@fn rawpcm_statcopy
 * @brief copies f_rpstat to an fstat sruct
 *
 * @param fstat[out] the bloated file stats struct
**/
void
rawpcm_statcopy(/*@out@*/ struct FileStats *const restrict fstat)
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

/**@fn opt_encode_rawpcm
 * @brief set raw PCM encoding
 *
 * @param optind0 the index of  'argv'
 * @param optind1 the index of *'argv'
 * @param argc unused
 * @param argv[in out] the argument vector from main()
 * @param mode unused
 *
 * @return 0
 *
 * @note rawpcm_statcopy() needs to be called from inside the mode
**/
// --rawpcm=format,samplerate,nchan
static int
opt_encode_rawpcm(
	const uint optind0, const uint optind1, UNUSED const uint argc,
	char *const *const argv, UNUSED const enum OptMode mode
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
{
	char *const opt = &argv[optind0][optind1];
	char *subopt;
	union {	longlong ll; } tmp;

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

	tmp.ll = atoll(subopt);
	if UNLIKELY ( (tmp.ll <= 0) || (tmp.ll > (longlong) UINT32_MAX) ){
		error_tta("%s: %s out of range: %lld",
			"--rawpcm", "samplerate", tmp.ll
		);
	}
	f_rpstat.samplerate = (u32) tmp.ll;

	// nchan
	subopt = strtok(NULL, ",");
	if UNLIKELY ( subopt == NULL ){
		error_tta("%s: missing %s field", "--rawpcm", "nchan");
	}
	assert(subopt != NULL);

	tmp.ll = atoll(subopt);
	if UNLIKELY ( (tmp.ll <= 0) || (tmp.ll > (longlong) UINT16_MAX) ){
		error_tta("%s: %s out of range: %lld",
			"--rawpcm", "nchan", tmp.ll
		);
	}
	f_rpstat.nchan = (u16) tmp.ll;

	// decfmt
	f_rpstat.decfmt = DECFMT_RAWPCM;

	g_flag.rawpcm = true;
	return 0;
}

/**@fn opt_encode_help
 * @brief print the mode_encode help to stderr and exit
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
opt_encode_help(
	UNUSED const uint optind0, UNUSED const uint optind1,
	UNUSED const uint argc, UNUSED char *const *const argv,
	UNUSED const enum OptMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	errprint_help_mode_encode();
	exit(EXIT_SUCCESS);
}

// EOF ///////////////////////////////////////////////////////////////////////
