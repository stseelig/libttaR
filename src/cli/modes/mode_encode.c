/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mode_encode.c                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>
#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../cli.h"
#include "../common.h"
#include "../debug.h"
#include "../formats.h"
#include "../main.h"
#include "../open.h"
#include "../opts.h"
#include "../system.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef seektable
#undef estat_out
#undef outfile
#undef infile
HOT
BUILD_EXTERN NOINLINE void encst_loop(
	struct SeekTable *RESTRICT seektable,
	/*@out@*/ struct EncStats *RESTRICT estat_out,
	const struct FileStats *RESTRICT, FILE *RESTRICT outfile,
	const char *, FILE *RESTRICT infile, const char *
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*seektable,
		*estat_out,
		outfile,
		infile
@*/
;

#undef seektable
#undef estat
#undef outfile
#undef infile
BUILD_EXTERN NOINLINE void encmt_loop(
	struct SeekTable *RESTRICT seektable,
	/*@out@*/ struct EncStats *RESTRICT estat_out,
	const struct FileStats *RESTRICT fstat, FILE *RESTRICT outfile,
	const char *, FILE *RESTRICT infile, const char *, unsigned int
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*seektable,
		*estat_out,
		outfile,
		infile
@*/
;

/* ======================================================================== */

static void enc_loop(const struct OpenedFilesMember *RESTRICT)
/*@globals	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
/*@modifies	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
;

/* //////////////////////////////////////////////////////////////////////// */

/**@fn mode_encode
 * @brief mode for encoding TTA
 *
 * @param optind - index of 'argv'
 * @param argc   - argument count from main()
 * @param argv   - argument vector from main()
 *
 * @return number of warnings/errors
**/
BUILD NOINLINE int
mode_encode(
	const unsigned int optind, const unsigned int argc,
	char *const *const argv
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		**argv
@*/
{
	struct OpenedFiles openedfiles;
	size_t nerrors_file = 0;
	timestamp_p ts_start, ts_finish;
	size_t i;
	union {	int d; } result;

	memset(&openedfiles, 0x00, sizeof openedfiles);

	timestamp_get(&ts_start);

	/* process opts/args */
	nerrors_file += optargs_process(
		&openedfiles, optind, argc, argv, &encode_optdict
	);

	/* get file stats */
	for ( i = 0; i < openedfiles.nmemb; ++i ){
		nerrors_file += filestats_get(
			openedfiles.file[i], MODE_ENCODE
		);
	}

	/* additional error check(s) */
	if UNLIKELY (
	     (openedfiles.nmemb > SIZE_C(1))
	    &&
	     (g_flag.outfile != NULL) && (! g_flag.outfile_is_dir)
	){
		warning_tta("multiple infiles, but outfile not a directory");
	}
	else if UNLIKELY ( openedfiles.nmemb == 0 ){
		warning_tta("nothing to do");
		nerrors_file += 1u;
	} else{;}

	/* exit if any errors */
	if UNLIKELY ( nerrors_file != 0 ){
		if ( nerrors_file > SIZE_C(255) ){
			nerrors_file = SIZE_C(255);
		}
		exit((int) nerrors_file);
	}

	/* encode each file */
	for ( i = 0; i < openedfiles.nmemb; ++i ){
		if ( (i != 0) && (! g_flag.quiet) ){
			(void) fputc('\n', stderr);
		}

		enc_loop(openedfiles.file[i]);

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

	/* print multifile stats */
	if ( (! g_flag.quiet) && (openedfiles.nmemb > SIZE_C(1)) ){
		timestamp_get(&ts_finish);
		errprint_runtime(
			timestamp_diff(&ts_start, &ts_finish),
			openedfiles.nmemb, MODE_ENCODE
		);
	}

	/* cleanup */
	openedfiles_close_free(&openedfiles);

	return (int) g_nwarnings;
}

/**@fn enc_loop
 * @brief prepares for and calls the encode loop function
 *
 * @param ofm - source file struct
**/
static void
enc_loop(const struct OpenedFilesMember *const RESTRICT ofm)
/*@globals	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
/*@modifies	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
{
	FILE *const RESTRICT   infile = ofm->infile;
	const char *const infile_name = ofm->infile_name;
	const struct FileStats *const RESTRICT fstat = &ofm->fstat;
	/* * */
	FILE *RESTRICT outfile = NULL;
	char *const RESTRICT outfile_name = get_outfile_name(
		infile_name, get_encfmt_sfx(fstat->encfmt)
	);
	/* * */
	const unsigned int nthreads = (g_nthreads != 0
		? g_nthreads : get_nprocessors_onln()
	);
	/* * */
	struct EncStats estat;
	struct SeekTable seektable;
	timestamp_p ts_start, ts_finish;
	union {	int	d; } result;
	union {	size_t	z; } tmp;

	/* pre-encode stats */
	if ( ! g_flag.quiet ){
		errprint_stats_precodec(
			fstat, infile_name, outfile_name, MODE_ENCODE
		);
	}

	/* setup seektable */
	switch ( fstat->encfmt ){
	default:
		assert(false);
		break;
	case xENCFMT_TTA1:
		/* seektable at start of file, size calculated in advance */
		tmp.z = seektable_nframes(
			fstat->decpcm_size, fstat->buflen,
			(unsigned int) fstat->samplebytes
		);
		seektable_init(&seektable, tmp.z);
		break;
	}
	/* MAYBE: handle file of unknown size */

	/* open outfile */
	outfile = fopen_check(outfile_name, "wb", FATAL);
	if UNLIKELY ( outfile == NULL ){
		error_sys(errno, "fopen", outfile_name);
	}
	assert(outfile != NULL);
	g_rm_on_sigint = outfile_name;

	/* save some space for the outfile header and seektable */
	/* MAYBE: handle file of unknown size */
	switch ( fstat->encfmt ){
	default:
		assert(false);
		break;
	case xENCFMT_TTA1:
		prewrite_tta1_header_seektable(
			outfile, &seektable, outfile_name
		);
		break;
	}

	/* seek to start of PCM */
	result.d = fseeko(infile, fstat->decpcm_off, SEEK_SET);
	if UNLIKELY ( result.d != 0 ){
		error_sys(errno, "fseeko", infile_name);
	}

	if ( ! g_flag.quiet ){
		timestamp_get(&ts_start);
	}

	/* encode */
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
		encst_loop(
			&seektable, &estat, fstat, outfile, outfile_name,
			infile, infile_name
		);
		break;
	case THREADMODE_MULTI:
encode_multi:
		encmt_loop(
			&seektable, &estat, fstat, outfile, outfile_name,
			infile, infile_name, nthreads
		);
		break;
	}

	/* write header and seektable */
	switch ( fstat->encfmt ){
	default:
		assert(false);
		break;
	case xENCFMT_TTA1:
		rewind(outfile);
		seektable.off = write_tta1_header(
			outfile, estat.nsamples_perchan, fstat, outfile_name
		);
		write_tta_seektable(outfile, &seektable, outfile_name);
		break;
	}

	if ( ! g_flag.quiet ){
		(void) fputs("C\r", stderr);
	}

	/* close outfile */
	result.d = fclose(outfile);
	if UNLIKELY ( result.d != 0 ){
		error_sys_nf(errno, "fclose", outfile_name);
	}
	g_rm_on_sigint = NULL;

	if ( ! g_flag.quiet ){
		timestamp_get(&ts_finish);
		estat.encodetime = timestamp_diff(&ts_start, &ts_finish);
	}

	/* post-encode stats */
	if ( ! g_flag.quiet ){
		errprint_stats_postcodec(fstat, &estat);
	}

	/* cleanup */
	free(outfile_name);
	seektable_free(&seektable);

	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
