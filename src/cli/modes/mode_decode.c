//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mode_decode.c                                                      //
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
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../bits.h"

#include "../cli.h"
#include "../debug.h"
#include "../formats.h"
#include "../main.h"
#include "../open.h"
#include "../opts.h"
#include "../system.h"

//////////////////////////////////////////////////////////////////////////////

#undef dstat_out
#undef outfile
#undef infile
void HOT decst_loop(
	const struct SeekTable *restrict,
	/*@out@*/ struct DecStats *restrict dstat_out,
	const struct FileStats *restrict, FILE *restrict outfile,
	const char *, FILE *restrict infile, const char *
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*dstat_out,
		outfile,
		infile
@*/
;

#undef dstat_out
#undef outfile
#undef infile
void decmt_loop(
	const struct SeekTable *restrict,
	/*@out@*/ struct DecStats *restrict dstat_out,
	const struct FileStats *restrict, FILE *restrict outfile,
	const char *, FILE *restrict infile, const char *, uint
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*dstat_out,
		outfile,
		infile
@*/
;

//////////////////////////////////////////////////////////////////////////////

static void dec_loop(struct OpenedFilesMember *restrict)
/*@globals	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
/*@modifies	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
;

//////////////////////////////////////////////////////////////////////////////

/**@fn mode_decode
 * @brief mode for decoding TTA
 *
 * @param optind the index of 'argv'
 * @param argc the argument count from main()
 * @param argv[in out] the argument vector from main()
 *
 * @return the number of warnings/errors
**/
int
mode_decode(const uint optind, const uint argc, char *const *const argv)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	struct OpenedFiles openedfiles;
	size_t nerrors_file = 0;
	timestamp_p ts_start, ts_finish;
		size_t i;
		union {	int d; } result;

	memset(&openedfiles, 0x00, sizeof openedfiles);

	timestamp_get(&ts_start);

	// process opts/args
	nerrors_file = optargs_process(
		&openedfiles, optind, argc, argv, &decode_optdict
	);

	// get file stats
	for ( i = 0; i < openedfiles.nmemb; ++i ){
		nerrors_file += filestats_get(
			openedfiles.file[i], MODE_DECODE
		);
	}

	// additional error check(s)
	if UNLIKELY (
	     (openedfiles.nmemb > (size_t) 1u)
	    &&
	     (g_flag.outfile != NULL) && (! g_flag.outfile_is_dir)
	){
		warning_tta("multiple infiles, but outfile not a directory");
	}
	else if UNLIKELY ( openedfiles.nmemb == 0 ){
		warning_tta("nothing to do");
		nerrors_file += 1u;
	} else{;}

	// exit if any errors
	if UNLIKELY ( nerrors_file != 0 ){
		if ( nerrors_file > (size_t) 255u ){
			nerrors_file = (size_t) 255u;
		}
		exit((int) nerrors_file);
	}

	// decode each file
	for ( i = 0; i < openedfiles.nmemb; ++i ){
		if ( (i != 0) && (! g_flag.quiet) ){
			(void) fputc('\n', stderr);
		}

		dec_loop(openedfiles.file[i]);

		result.d = fclose(openedfiles.file[i]->infile);
		assert(result.d == 0);
		openedfiles.file[i]->infile = NULL;

		if ( g_flag.delete_src ){
			result.d = remove(openedfiles.file[i]->infile_name);
			if UNLIKELY ( result.d != 0 ){
				error_sys_nf(
					errno, "remove",
					openedfiles.file[i]->infile_name
				);
			}
		}
	}

	// print multifile stats
	if ( (! g_flag.quiet) && (openedfiles.nmemb > (size_t) 1u) ){
		timestamp_get(&ts_finish);
		errprint_runtime(
			timestamp_diff(&ts_start, &ts_finish),
			openedfiles.nmemb, MODE_DECODE
		);
	}

	// cleanup
	openedfiles_close_free(&openedfiles);

	return (int) g_nwarnings;
}

/**@fn dec_loop
 * @brief prepares for and calls the decode loop function
 *
 * @param ofm[in] the source file struct
**/
static void
dec_loop(struct OpenedFilesMember *const restrict ofm)
/*@globals	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
/*@modifies	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
{
	FILE *const restrict infile = ofm->infile;
	const char *const infile_name = ofm->infile_name;
	struct FileStats *const restrict fstat = &ofm->fstat;
	//
	FILE *restrict outfile = NULL;
	char *const restrict outfile_name = get_outfile_name(
		infile_name, get_decfmt_sfx(g_flag.decfmt)
	);
	//
	const uint nthreads = (g_nthreads != 0
		? g_nthreads : get_nprocessors_onln()
	);
	//
	struct DecStats dstat;
	struct SeekTable seektable;
	bool ignore_seektable = false;
	timestamp_p ts_start, ts_finish;
	union {	int		d;
		enum FileCheck	fc;
	} result;
	union {	size_t z; } tmp;

	fstat->decfmt = g_flag.decfmt;
	//
	switch ( fstat->samplebytes ){
	default:
		assert(false);
		break;
	case LIBTTAr_SAMPLEBYTES_1:
		fstat->inttype = INT_UNSIGNED;
		break;
	case LIBTTAr_SAMPLEBYTES_2:
	case LIBTTAr_SAMPLEBYTES_3:
		fstat->inttype = INT_SIGNED;
		break;
	}
	//
	fstat->endian = xENDIAN_LITTLE;	// wav/w64 only LE

	// pre-decode stats
	if ( ! g_flag.quiet ){
		errprint_stats_precodec(
			fstat, infile_name, outfile_name, MODE_DECODE
		);
	}

	// seek to seektable
	// already there for TTA1

	// copy/check seektable
	tmp.z  = fstat->nsamples_enc + fstat->framelen - 1u;
	tmp.z /= fstat->framelen;
	result.fc = filecheck_tta_seektable(&seektable, tmp.z, infile);
	if UNLIKELY ( result.fc != FILECHECK_OK ){
		if ( result.fc == FILECHECK_CORRUPTED ){
			ignore_seektable = true;
			warning_tta("%s: corrupted seektable", infile_name);
		}
		else {	error_filecheck(result.fc, errno, fstat, infile_name);
			exit(result.fc);
		}
	}
	// MAYBE check that the combined seektable entries matches filesize

	// open outfile
	outfile = fopen_check(outfile_name, "wb", FATAL);
	if UNLIKELY ( outfile == NULL ){
		error_sys(errno, "fopen", outfile_name);
	}
	assert(outfile != NULL);
	//
	g_rm_on_sigint = outfile_name;

	// save some space for the outfile header
	switch ( fstat->decfmt ){
	default:
		assert(false);
		break;
	case DECFMT_RAWPCM:
		break;
	case DECFMT_WAV:
		prewrite_wav_header(outfile, outfile_name);
		break;
	case DECFMT_W64:
		prewrite_w64_header(outfile, outfile_name);
		break;
	}

	// seek to tta data
	// already there for TTA1

	if ( ! g_flag.quiet ){
		timestamp_get(&ts_start);
	}

	// decode
	switch ( g_flag.threadmode ){
	default:
		assert(false);
		break;
	case THREADMODE_UNSET:
		if ( nthreads > 1u ){
			goto encode_multi;
		}
		/*@fallthrough@*/
	case THREADMODE_SINGLE:
		decst_loop(
			&seektable, &dstat, fstat, outfile, outfile_name,
			infile, infile_name
		);
		break;
	case THREADMODE_MULTI:
encode_multi:
		decmt_loop(
			&seektable, &dstat, fstat, outfile, outfile_name,
			infile, infile_name, nthreads
		);
		break;
	}

	if UNLIKELY (
		(! ignore_seektable) && (dstat.nframes != seektable.nmemb)
	){
		warning_tta("%s: truncated file / malformed seektable",
			infile_name
		);
	}

	// update header
	switch ( fstat->decfmt ){
	default:
		assert(false);
		break;
	case DECFMT_RAWPCM:
		break;
	case DECFMT_W64:
		rewind(outfile);
		write_w64_header(
			outfile,
			(size_t) (dstat.nsamples_flat * fstat->samplebytes),
			fstat, outfile_name
		);
		break;
	case DECFMT_WAV:
		rewind(outfile);
		write_wav_header(
			outfile,
			(size_t) (dstat.nsamples_flat * fstat->samplebytes),
			fstat, outfile_name
		);
		break;
	}

	if ( ! g_flag.quiet ){
		(void) fputs("C\r", stderr);
	}

	// close outfile
	result.d = fclose(outfile);
	if UNLIKELY ( result.d != 0 ){
		error_sys_nf(errno, "fclose", outfile_name);
	}
	//
	g_rm_on_sigint = NULL;

	if ( ! g_flag.quiet ){
		timestamp_get(&ts_finish);
		dstat.decodetime = timestamp_diff(&ts_start, &ts_finish);
	}

	// post-decode stats
	if ( ! g_flag.quiet ){
		errprint_stats_postcodec(fstat, (struct EncStats *) &dstat);
	}

	// cleanup
	free(outfile_name);
	seektable_free(&seektable);

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
