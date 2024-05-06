//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// tta2dec.c                                                                //
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

#include "../bits.h"
#include "../libttaR.h"
#include "../splint.h"

#include "bufs.h"
#include "cli.h"
#include "debug.h"
#include "formats.h"
#include "main.h"
#include "open.h"
#include "opts.h"
#include "tta2.h"

//////////////////////////////////////////////////////////////////////////////

static void tta2dec_loop(struct OpenedFilesMember *const restrict)
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
tta2dec(uint optind)
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
	union {
		int	d;
		bool	b;
	} t;

	memset(&openedfiles, 0x00, sizeof openedfiles);

	(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);

	// process opts/args
	nerrors_file = optargs_process(
		&openedfiles, optind, tta2dec_optdict
	);

	//// program intro
	//if ( ! g_flag.quiet ){
	//	errprint_program_intro();
	//}

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

		if ( ! libttaR_test_nchan((uint) ofm->fstat.nchan) ){
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
	if ( (g_flag.outfile != NULL)
	    &&
	     (openedfiles.nmemb > (size_t) 1)
	    &&
	     (! g_flag.outfile_is_dir)
	){
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
		tta2dec_loop(openedfiles.file[i]);
		(void) fclose(openedfiles.file[i]->infile);
		openedfiles.file[i]->infile = NULL;
		if ( g_flag.delete_src ){
			t.d = remove(openedfiles.file[i]->infile_name);
			if ( t.d != 0 ){
				error_sys_nf(
					errno, "remove", strerror(errno),
					openedfiles.file[i]->infile_name
				);
			}
		}
	}
	if ( openedfiles.nmemb == 0 ){
		warning_tta("nothing to do");
	}

	// print multifile stats
	if ( (! g_flag.quiet) && (openedfiles.nmemb > (size_t) 1) ){
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_stop);
		errprint_runtime(
			timediff(&ts_start, &ts_stop), openedfiles.nmemb,
			MODE_DECODE
		);
	}

//#ifndef NDEBUG
	// cleanup
	openedfiles_close_free(&openedfiles);
//#endif
	return (int) g_nwarnings;
}

static void
tta2dec_loop(struct OpenedFilesMember *const restrict ofm)
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
	const char *const restrict infile_name = ofm->infile_name;
	struct FileStats *const restrict fstat = &ofm->fstat;
	//
	FILE *restrict outfile = NULL;
	char *outfile_name = NULL;
	const char *outfile_dir  = NULL;
	const char *outfile_sfx  = NULL;
	//
	struct LibTTAr_CodecState_Priv *state_priv;
	struct LibTTAr_CodecState_User state_user;
	//
	struct DecStats dstat;
	struct SeekTable seektable;
	struct DecBuf decbuf;
	bool ignore_seektable = false;
	struct timespec ts_start, ts_stop;
	union {
		size_t		z;
		enum FileCheck	fc;
	} t;

	// set outfile_name
	fstat->decfmt = g_flag.decfmt;
	if ( g_flag.outfile != NULL ){
		if ( g_flag.outfile_is_dir ){
			outfile_dir  = g_flag.outfile;
			outfile_name = (char *) infile_name;
			outfile_sfx  = get_outfile_sfx(fstat->decfmt);
		}
		else {	outfile_name = (char *) g_flag.outfile;
		}
	}
	else {	outfile_name = (char *) infile_name;
		outfile_sfx  = get_outfile_sfx(fstat->decfmt);
	}
	outfile_name = outfile_name_fmt(
		outfile_dir, outfile_name, outfile_sfx
	);
	g_rm_on_sigint = outfile_name;
	//
	switch ( fstat->samplebytes ){
	case 1u:
		fstat->inttype = INT_UNSIGNED;
		break;
	case 2u:
	case 3u:
		fstat->inttype = INT_SIGNED;
		break;
	}
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
	if ( t.fc != FILECHECK_OK ){
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
	if ( outfile == NULL ){
		error_sys(errno, "fopen", strerror(errno), outfile_name);
	}
	assert(outfile != NULL);

	// save some space for the outfile header
	switch ( fstat->decfmt ){
	case FORMAT_RAWPCM:
		break;
	case FORMAT_WAV:
		prewrite_wav_header(outfile, outfile_name);
		break;
	case FORMAT_W64:
		prewrite_w64_header(outfile, outfile_name);
		break;
	}

	// seek to tta data
	// already there for TTA1

	// setup buffers
	t.z = decbuf_init(
		&decbuf, g_samplebuf_len, (uint) fstat->nchan,
		fstat->samplebytes
	);
	assert(t.z == g_samplebuf_len * fstat->nchan);
	//
	t.z = libttaR_codecstate_priv_size((uint) fstat->nchan);
	assert(t.z != 0);
	state_priv = malloc(t.z);
	if ( state_priv == NULL ){
		error_sys(errno, "malloc", strerror(errno), NULL);
	}
	assert(state_priv != NULL);

	// decode
	if ( ! g_flag.quiet ){
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);
	}
	ttadec_loop_st(
		state_priv, &state_user, &decbuf, &seektable, &dstat, fstat,
		outfile, outfile_name, infile, infile_name
	);
	if ( ! g_flag.quiet ){
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_stop);
		dstat.decodetime += timediff(&ts_start, &ts_stop);
	}

	if ( (! ignore_seektable) && (dstat.nframes != seektable.nmemb) ){
		warning_tta("%s: truncated file / malformed seektable",
			infile_name
		);
	}

	// update header
	switch ( fstat->decfmt ){
	case FORMAT_RAWPCM:
		break;
	case FORMAT_W64:
		rewind(outfile);
		write_w64_header(
			outfile,
			(size_t) (dstat.nsamples * fstat->samplebytes), fstat,
			outfile_name
		);
		break;
	case FORMAT_WAV:
		rewind(outfile);
		write_wav_header(
			outfile,
			(size_t) (dstat.nsamples * fstat->samplebytes), fstat,
			outfile_name
		);
		break;
	}

	// post-decode stats
	if ( ! g_flag.quiet ){
		errprint_stats_postcodec(fstat, (struct EncStats *) &dstat);
	}

	// cleanup
	if ( outfile != NULL ){
		(void) fclose(outfile);
	}
	g_rm_on_sigint = NULL;
	if ( outfile_name != NULL ){
		free(outfile_name);
	}
	if ( state_priv != NULL ){
		free(state_priv);
	}
	decbuf_free(&decbuf);
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
	union {
		enum FileCheck	fc;
	} t;

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

//==========================================================================//

// returns nmemb of buf zero-padded
uint
ttadec_frame_zeropad(
	i32 *const restrict buf,
	struct LibTTAr_CodecState_User *const restrict user, uint diff,
	uint nchan
)
/*@modifies	*buf,
		user->ni32_perframe,
		user->ni32,
		user->ni32_total
@*/
{
	const size_t r = (size_t) (nchan - diff);

	memset(&buf[user->ni32], 0x00, r * (sizeof *buf));

	user->ni32_perframe	+= r;
	user->ni32		+= r;
	user->ni32_total	+= r;
	return (uint) r;
}

// EOF ///////////////////////////////////////////////////////////////////////
