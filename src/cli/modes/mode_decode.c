//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mode_decode.c                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <time.h>
#include <unistd.h>

#include "../../bits.h"
#include "../../splint.h"

#include "../cli.h"
#include "../debug.h"
#include "../formats.h"
#include "../main.h"
#include "../open.h"
#include "../opts.h"

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
	uint nerrors_file = 0;
	struct timespec ts_start, ts_stop;
		size_t i;
		union {	int	d;
			bool	b;
		} t;

	memset(&openedfiles, 0x00, sizeof openedfiles);

	t.d = clock_gettime(CLOCK_MONOTONIC, &ts_start);
	assert(t.d == 0);

	// process opts/args
	nerrors_file = optargs_process(
		&openedfiles, optind, argc, argv, decode_optdict
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
		++nerrors_file;
	} else{;}

	// exit if any errors
	if UNLIKELY ( nerrors_file != 0 ){
		exit((int) nerrors_file);
	}

	// decode each file
	for ( i = 0; i < openedfiles.nmemb; ++i ){
		if ( (i != 0) && (! g_flag.quiet) ){
			(void) fputc('\n', stderr);
		}

		dec_loop(openedfiles.file[i]);

		t.d = fclose(openedfiles.file[i]->infile);
		assert(t.d == 0);
		openedfiles.file[i]->infile = NULL;

		if ( g_flag.delete_src ){
			t.d = remove(openedfiles.file[i]->infile_name);
			if UNLIKELY ( t.d != 0 ){
				error_sys_nf(
					errno, "remove",
					openedfiles.file[i]->infile_name
				);
			}
		}
	}

	// print multifile stats
	if ( (! g_flag.quiet) && (openedfiles.nmemb > (size_t) 1u) ){
		t.d = clock_gettime(CLOCK_MONOTONIC, &ts_stop);
		assert(t.d == 0);
		errprint_runtime(
			timediff(&ts_start, &ts_stop), openedfiles.nmemb,
			MODE_DECODE
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
		? g_nthreads : (uint) sysconf(_SC_NPROCESSORS_ONLN)
	);
	//
	struct DecStats dstat;
	struct SeekTable seektable;
	bool ignore_seektable = false;
	struct timespec ts_start, ts_stop;
	union {	size_t		z;
		int		d;
		enum FileCheck	fc;
	} t;

	fstat->decfmt = g_flag.decfmt;
	//
	switch ( fstat->samplebytes ){
	case TTASAMPLEBYTES_1:
		fstat->inttype = INT_UNSIGNED;
		break;
	case TTASAMPLEBYTES_2:
	case TTASAMPLEBYTES_3:
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
	t.z = (fstat->nsamples_enc + fstat->framelen - 1u) / fstat->framelen;
	t.fc = filecheck_tta_seektable(&seektable, t.z, infile);
	if UNLIKELY ( t.fc != FILECHECK_OK ){
		if ( t.fc == FILECHECK_CORRUPTED ){
			ignore_seektable = true;
			warning_tta("%s: corrupted seektable", infile_name);
		}
		else {	error_filecheck(t.fc, errno, fstat, infile_name);
			exit(t.fc);
		}
	}
	// MAYBE check that the combined seektable entries matches filesize

	// open outfile
	outfile = fopen_check(outfile_name, "w", FATAL);
	if UNLIKELY ( outfile == NULL ){
		error_sys(errno, "fopen", outfile_name);
	}
	assert(outfile != NULL);
	//
	g_rm_on_sigint = outfile_name;

	// save some space for the outfile header
	switch ( fstat->decfmt ){
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
		t.d = clock_gettime(CLOCK_MONOTONIC, &ts_start);
		assert(t.d == 0);
	}

	// decode
	switch ( g_flag.threadmode ){
	case THREADMODE_UNSET:
		if ( nthreads > 1u ){ goto encode_multi; }
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
	t.d = fclose(outfile);
	if UNLIKELY ( t.d != 0 ){
		error_sys_nf(errno, "fclose", outfile_name);
	}
	//
	g_rm_on_sigint = NULL;

	if ( ! g_flag.quiet ){
		t.d = clock_gettime(CLOCK_MONOTONIC, &ts_stop);
		assert(t.d == 0);
		dstat.decodetime += timediff(&ts_start, &ts_stop);
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
