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

#include "../../bits.h"
#include "../../libttaR.h"
#include "../../splint.h"

#include "../cli.h"
#include "../debug.h"
#include "../formats.h"
#include "../main.h"
#include "../open.h"
#include "../opts.h"

#include "bufs.h"
#include "tta2.h"

//////////////////////////////////////////////////////////////////////////////

static void dec_loop(struct OpenedFilesMember *const restrict)
/*@globals	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
/*@modifies	fileSystem,
		internalState,
		g_rm_on_sigint
@*/
;

#undef fstat
#undef seektable
#undef file
static enum FileCheck filecheck_encfmt(
	struct FileStats *const restrict fstat, FILE *const restrict file,
	const char *const restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		fstat,
		file
@*/
;

//////////////////////////////////////////////////////////////////////////////

int
mode_decode(uint optind)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	struct OpenedFiles openedfiles;
	struct OpenedFilesMember *ofm;
	uint nerrors_file = 0;
	struct timespec ts_start, ts_stop;
	size_t i;
	union {	int	d;
		bool	b;
	} t;

	memset(&openedfiles, 0x00, sizeof openedfiles);

	(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);

	// process opts/args
	nerrors_file = optargs_process(
		&openedfiles, optind, tta2dec_optdict
	);

	// get file stats
	for ( i = 0; i < openedfiles.nmemb; ++i ){

		ofm = openedfiles.file[i];
		if ( ofm->infile == NULL ){ continue; } // bad filename

		// check for supported filetypes and fill most of fstat
		t.b = (bool) filecheck_encfmt(
			&ofm->fstat, ofm->infile, ofm->infile_name
		);
		if ( t.b ){
			++nerrors_file;
			continue;
		}

		if UNLIKELY ( ! libttaR_test_nchan((uint) ofm->fstat.nchan) ){
			++nerrors_file;
			error_tta_nf("%s: libttaR built without support for"
				" nchan of %u", ofm->infile_name,
				ofm->fstat.nchan
			);
		}

		// the rest of fstat
		//ofm->fstat.encfmt      = FORMAT_TTA1;
		ofm->fstat.framelen    = libttaR_nsamples_perframe(
			ofm->fstat.samplerate
		);
		ofm->fstat.buflen      = (size_t) (
			ofm->fstat.framelen * ofm->fstat.nchan
		);
		ofm->fstat.samplebytes = (enum TTASampleBytes) (
			(ofm->fstat.samplebits + 7u) / 8u
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
	if ( nerrors_file != 0 ){
		exit((int) nerrors_file);
	}

	// decode each file
	for ( i = 0; i < openedfiles.nmemb; ++i ){
		if ( (i != 0) && (! g_flag.quiet) ){
			(void) fputc('\n', stderr);
		}
		//
		dec_loop(openedfiles.file[i]);
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
	if UNLIKELY ( openedfiles.nmemb == 0 ){
		warning_tta("nothing to do");
	}

	// print multifile stats
	if ( (! g_flag.quiet) && (openedfiles.nmemb > (size_t) 1u) ){
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_stop);
		errprint_runtime(
			timediff(&ts_start, &ts_stop), openedfiles.nmemb,
			MODE_DECODE
		);
	}

	// cleanup
	openedfiles_close_free(&openedfiles);

	return (int) g_nwarnings;
}

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
	t.z = (fstat->nsamples + fstat->framelen - 1u) / fstat->framelen;
	t.fc = filecheck_tta_seektable(&seektable, t.z, infile);
	if UNLIKELY ( t.fc != FILECHECK_OK ){
		if ( t.fc == FILECHECK_CORRUPTED ){
			ignore_seektable = true;
			warning_tta("%s: corrupted seektable", infile_name);
		}
		else {	error_filecheck(t.fc, fstat, infile_name, errno);
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
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);
	}

	// decode
	memset(&dstat, 0x00, sizeof dstat);
	ttadec_loop_st(
		&seektable, &dstat, fstat, outfile, outfile_name, infile,
		infile_name
	);

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
			(size_t) (dstat.nsamples * fstat->samplebytes), fstat,
			outfile_name
		);
		break;
	case DECFMT_WAV:
		rewind(outfile);
		write_wav_header(
			outfile,
			(size_t) (dstat.nsamples * fstat->samplebytes), fstat,
			outfile_name
		);
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
		dstat.decodetime += timediff(&ts_start, &ts_stop);
	}

	// post-decode stats
	if ( ! g_flag.quiet ){
		errprint_stats_postcodec(fstat, (struct EncStats *) &dstat);
	}

	// cleanup
	g_rm_on_sigint = NULL;
	free(outfile_name);
	seektable_free(&seektable);
}

static enum FileCheck
filecheck_encfmt(
	struct FileStats *const restrict fstat,	FILE *const restrict file,
	const char *const restrict filename
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		fstat,
		file
@*/
{
	union {	enum FileCheck	fc; } t;

	// seek past any metadata on the input file
	t.fc = metatags_skip(file);
	if ( t.fc != FILECHECK_MISMATCH ){
		error_filecheck(t.fc, fstat, filename, errno);
		return t.fc;
	}

	// TTA1
	t.fc = filecheck_tta1(fstat, file);
	if ( t.fc == FILECHECK_OK ){ goto end_check; }
	error_filecheck(t.fc, fstat, filename, errno);
	return t.fc;

end_check:
	// check that file stats are within bounds / reasonable
	if ( (fstat->nchan == 0)
	    ||
	     (fstat->samplerate == 0)
	    ||
	     (fstat->samplebits == 0)
	    ||
	     (fstat->samplebits > (u16) TTA_SAMPLEBITS_MAX)
	){
		error_filecheck(
			FILECHECK_UNSUPPORTED_RESOLUTION, fstat, filename,
			errno
		);
		return FILECHECK_UNSUPPORTED_RESOLUTION;
	}
	return FILECHECK_OK;
}

// EOF ///////////////////////////////////////////////////////////////////////
