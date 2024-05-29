//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/mode_encode.c                                                      //
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

#include "bufs.h"

//////////////////////////////////////////////////////////////////////////////

#undef seektable
#undef estat_out
#undef outfile
#undef infile
extern HOT void encst_loop(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct EncStats *const restrict estat_out,
	const struct FileStats *const restrict,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const
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
extern void encmt_loop(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct EncStats *const restrict estat_out,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const, uint
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

//////////////////////////////////////////////////////////////////////////////

static void enc_loop(const struct OpenedFilesMember *const restrict)
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

int
mode_encode(uint optind)
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

	(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);

	// process opts/args
	nerrors_file += optargs_process(
		&openedfiles, optind, encode_optdict
	);

	// get file stats
	for ( i = 0; i < openedfiles.nmemb; ++i ){
		nerrors_file += filestats_get(
			openedfiles.file[i], MODE_ENCODE
		);
	}

	// additional error check(s)
	if UNLIKELY (
	     (g_flag.outfile != NULL)
	    &&
	     (openedfiles.nmemb > (size_t) 1u)
	    &&
	     (! g_flag.outfile_is_dir)
	){
		// MAYBE just warn
		error_tta_nf("multiple infiles, but outfile not a directory");
		++nerrors_file;
	}

	// exit if any errors
	if UNLIKELY ( nerrors_file != 0 ){
		exit((int) nerrors_file);
	}
	//
	if UNLIKELY ( openedfiles.nmemb == 0 ){
		warning_tta("nothing to do");
	}

	// encode each file
	for ( i = 0; i < openedfiles.nmemb; ++i ){
		if ( (i != 0) && (! g_flag.quiet) ){
			(void) fputc('\n', stderr);
		}
		//
		enc_loop(openedfiles.file[i]);
		//
		(void) fclose(openedfiles.file[i]->infile);
		openedfiles.file[i]->infile = NULL;
		//
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
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_stop);
		errprint_runtime(
			timediff(&ts_start, &ts_stop), openedfiles.nmemb,
			MODE_ENCODE
		);
	}

	// cleanup
	openedfiles_close_free(&openedfiles);

	return (int) g_nwarnings;
}

static void
enc_loop(const struct OpenedFilesMember *const restrict ofm)
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
	const struct FileStats *const restrict fstat = &ofm->fstat;
	//
	FILE *restrict outfile = NULL;
	char *const restrict outfile_name = get_outfile_name(
		infile_name, get_encfmt_sfx(fstat->encfmt)
	);
	//
	const uint nthreads = (g_nthreads != 0
		? g_nthreads : (uint) sysconf(_SC_NPROCESSORS_ONLN)
	);
	//
	struct EncStats estat;
	struct SeekTable seektable;
	struct timespec ts_start, ts_stop;
	union {	size_t	z;
		int	d;
	} t;

	// pre-encode stats
	if ( ! g_flag.quiet ){
		errprint_stats_precodec(
			fstat, infile_name, outfile_name, MODE_ENCODE
		);
	}

	// setup seektable
	switch ( fstat->encfmt ){
	case xENCFMT_TTA1:
		// seektable at start of file, size calculated in advance
		t.z  = (fstat->decpcm_size + fstat->buflen) / fstat->buflen;
		t.z -= (size_t) 1u;
		t.z  = (size_t) (t.z + fstat->samplebytes);
		t.z /= (size_t) fstat->samplebytes;
		seektable_init( &seektable, t.z);
		break;
	}
	// TODO handle file of unknown size
	//else {	// TODO setup tmpfile for writing
	//	seektable_init(&seektable, 0);
	//}

	// open outfile
	outfile = fopen_check(outfile_name, "w", FATAL);
	if UNLIKELY ( outfile == NULL ){
		error_sys(errno, "fopen", outfile_name);
	}
	assert(outfile != NULL);
	//
	g_rm_on_sigint = outfile_name;

	// save some space for the outfile header and seektable
	// TODO handle file of unknown size
	switch ( fstat->encfmt ){
	case xENCFMT_TTA1:
		prewrite_tta1_header_seektable(
			outfile, &seektable, outfile_name
		);
		break;
	}

	// seek to start of pcm
	(void) fseeko(infile, fstat->decpcm_off, SEEK_SET);

	if ( ! g_flag.quiet ){
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);
	}

	// encode
	switch ( g_flag.threadmode ){
	case THREADMODE_UNSET:
		if ( nthreads > 1u ){ goto encode_multi; }
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
			infile, infile_name, (uint) nthreads
		);
		break;
	}

	// write header and seektable
	switch ( fstat->encfmt ){
	case xENCFMT_TTA1:
		rewind(outfile);
		seektable.off = write_tta1_header(
			outfile, estat.nsamples_perchan, fstat, outfile_name
		);
		write_tta_seektable(outfile, &seektable, outfile_name);
		break;
	}

	// close outfile
	if ( ! g_flag.quiet ){
		(void) fputs("C\r", stderr);
	}
	t.d = fclose(outfile);
	if UNLIKELY ( t.d != 0 ){
		error_sys_nf(errno, "fclose", outfile_name);
	}

	if ( ! g_flag.quiet ){
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_stop);
		estat.encodetime += timediff(&ts_start, &ts_stop);
	}

	// post-encode stats
	if ( ! g_flag.quiet ){
		errprint_stats_postcodec(fstat, &estat);
	}

	// cleanup
	g_rm_on_sigint = NULL;
	free(outfile_name);
	seektable_free(&seektable);

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
