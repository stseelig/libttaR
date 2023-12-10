//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// tta2dec.c                                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
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

#undef priv
#undef user
#undef decbuf
#undef outfile
#undef infile
static uint ttadec_frame(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	struct LibTTAr_CodecState_User *const restrict user,
	const struct DecBuf *const restrict decbuf,
	const struct FileStats *const restrict,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const, size_t, size_t
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*priv,
		*user,
		*decbuf->i32buf,
		*decbuf->ttabuf,
		outfile,
		infile
@*/
;

#undef readlen
#undef ni32_target
#undef infile
static void
ttadec_frame_adjust(
	size_t *const restrict readlen, size_t *const restrict ni32_target,
	const struct LibTTAr_CodecState_User *const restrict,
	FILE *const restrict infile, const char *const, size_t
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*readlen,
		*ni32_target,
		infile
@*/
;

#undef buf
#undef user
static uint ttadec_frame_zeropad(
	i32 *const restrict buf,
	struct LibTTAr_CodecState_User *const restrict user, uint, uint
)
/*@modifies	*buf,
		user->ni32_perframe,
		user->ni32,
		user->ni32_total
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
		int d;
	} t;

	(void) memset(&openedfiles, 0x00, sizeof openedfiles);

	(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);

	// process opts/args
	nerrors_file = optargs_process(
		&openedfiles, optind, tta2dec_optdict
	);

	// program intro
	if ( ! g_flag.quiet ){
		errprint_program_intro();
	}

	// get file stats
	for ( i = 0; i < openedfiles.nmemb; ++i ){

		ofm = openedfiles.file[i];
		if ( ofm->infile == NULL ){ continue; } // bad filename

		// check for supported filetypes and fill most of fstat
		nerrors_file += (uint) ((bool) filecheck_encfmt(
			&ofm->fstat, ofm->infile, ofm->infile_name
		));
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

#ifndef NDEBUG
	// cleanup
	openedfiles_close_free(&openedfiles);
#endif
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
	size_t framesize_tta;
	uint dec_retval;
	u32 crc_read;
	union {
		size_t		z;
		int		d;
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
			error_tta_nf("%s: corrupted seektable", infile_name);
			// TODO warn and handle instead of error
			//warning_tta("%s: corrupted seektable", infile_name);
		}
		else {	error_filecheck(t.fc, fstat, infile_name, errno);
			exit(t.fc);
		}
	}
	// MAYBE check that the combined seektable entries matches filesize
	//  individual checked in decbuf_adjust

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
		prewrite_wav_header(outfile);
		break;
	case FORMAT_W64:
		prewrite_w64_header(outfile);
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
	state_priv = malloc(
		libttaR_codecstate_priv_size((uint) fstat->nchan)
	);
	if ( state_priv == NULL ){
		error_sys(errno, "malloc", strerror(errno), NULL);
	}
	assert(state_priv != NULL);
	//
	(void) memset(&state_user, 0x00, sizeof state_user);
	state_user.ni32_perframe = fstat->buflen;

	if ( ! g_flag.quiet ){
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);
	}

	// decode loop
	(void) memset(&dstat, 0x00, sizeof dstat);
	do {
		if ( (! g_flag.quiet) && (dstat.nframes % (size_t) 64 == 0) ){
			errprint_spinner();
		}

		// calc framelen and adjust if necessary
		t.z = dstat.nsamples_perchan + fstat->framelen;
		if ( t.z > fstat->nsamples ){
			t.z = fstat->nsamples - dstat.nsamples_perchan;
			state_user.ni32_perframe = t.z * fstat->nchan;
		}
		//
		framesize_tta  = letoh32(seektable.table[dstat.nframes]);
		framesize_tta -= (sizeof crc_read);

		// decode and write frame
		dec_retval = ttadec_frame(
			state_priv, &state_user, &decbuf, fstat, outfile,
			outfile_name, infile, infile_name, dstat.nframes,
			framesize_tta
		);

		// check frame crc
		t.z = fread(&crc_read, (size_t) 1, sizeof crc_read, infile);
		if ( t.z != sizeof crc_read ){
			if ( feof(infile) != 0 ){
				error_sys_nf(
					errno, "fread", strerror(errno),
					infile_name
				);
			}
			else {	warning_tta("%s: truncated file /"
					" malformed seektable", infile_name
				);
				// TODO retry decoding without seektable
			}
		}
		state_user.nbytes_tta_total += sizeof crc_read;
		if ( state_user.crc != letoh32(crc_read) ){
			warning_tta("%s: frame %zu is corrupted; bad crc",
				infile_name, dstat.nframes
			);
		}

		dstat.nframes		+= (size_t) 1;
		dstat.nsamples		+= state_user.ni32_total;
		dstat.nsamples_perchan	+= (size_t) (
			state_user.ni32_total / fstat->nchan
		);
		dstat.nbytes_decoded	+= state_user.nbytes_tta_total;
	}
	while ( (state_user.ni32_total == fstat->buflen)
	       &&
	        (dec_retval == 0)
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
			(size_t) (dstat.nsamples * fstat->samplebytes), fstat
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

// returns 0 on OK, or number of bytes padded
static uint
ttadec_frame(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	struct LibTTAr_CodecState_User *const restrict user,
	const struct DecBuf *const restrict decbuf,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name,
	size_t frame_num, size_t framesize_tta
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*priv,
		*user,
		*decbuf->i32buf,
		*decbuf->ttabuf,
		outfile,
		infile
@*/
{
	uint r = 0;
	size_t readlen, nbytes_read, ni32_target;
	union {
		size_t	z;
		uint	u;
		int	d;
	} t;

	user->is_new_frame = true;
	ni32_target = (decbuf->i32buf_len < user->ni32_perframe
		? decbuf->i32buf_len
		: user->ni32_perframe
	);
	readlen = (decbuf->ttabuf_len < framesize_tta
		? decbuf->ttabuf_len
		: framesize_tta
	);
	do {
		// read tta from infile
		nbytes_read = fread(
			decbuf->ttabuf, (size_t) 1, readlen, infile
		);
		if ( nbytes_read != readlen ){
			if ( feof(infile) != 0 ){
				error_sys_nf(
					errno, "fread", strerror(errno),
					infile_name
				);
			}
			else {	warning_tta("%s: truncated file",
					infile_name
				);
			}
		}

		// decode tta to i32
		t.d = libttaR_tta_decode(
			decbuf->i32buf, decbuf->ttabuf, decbuf->i32buf_len,
			decbuf->ttabuf_len, ni32_target, nbytes_read, priv,
			user, fstat->samplebytes, (uint) fstat->nchan
		);
		assert(t.d == 0);

		// check for truncated sample
		if ( user->frame_is_finished ){
			t.u = (uint) (user->ni32_total % fstat->nchan);
			if ( t.u != 0 ){
				warning_tta("%s: frame %zu: "
					"last sample truncated, zero-padding",
					infile_name, frame_num
				);
				r = ttadec_frame_zeropad(
					decbuf->i32buf, user, t.u,
					(uint) fstat->nchan
				);
			}
		}

		// convert i32 to pcm
		t.z = libttaR_pcm_write(
			decbuf->pcmbuf, decbuf->i32buf, user->ni32,
			fstat->samplebytes
		);
		assert(t.z == user->ni32);

		// write pcm to outfile
		t.z = fwrite(
			decbuf->pcmbuf, fstat->samplebytes, user->ni32,
			outfile
		);
		if ( t.z != user->ni32 ){
			error_sys_nf(
				errno, "fwrite", strerror(errno), outfile_name
			);
		}

		if ( r != 0 ){ break; }
		if ( ! user->frame_is_finished ){
			ttadec_frame_adjust(
				&readlen, &ni32_target, user, infile,
				infile_name, framesize_tta
			);
		}
	}
	while ( ! user->frame_is_finished );

	return r;
}

// returns nmemb of buf zero-padded
static uint
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

	(void) memset(&buf[user->ni32], 0x00, r * (sizeof *buf));

	user->ni32_perframe	+= r;
	user->ni32		+= r;
	user->ni32_total	+= r;
	return (uint) r;
}

static void
ttadec_frame_adjust(
	size_t *const restrict readlen, size_t *const restrict ni32_target,
	const struct LibTTAr_CodecState_User *const restrict user,
	FILE *const restrict infile, const char *const infile_name,
	size_t framesize_tta
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*readlen,
		*ni32_target,
		infile
@*/
{
	union {
		off_t	o;
		int	d;
	} t;

	// seek infile to first non-decoded byte
	if ( user->nbytes_tta < *readlen ){
		t.o  = (off_t) (*readlen - user->nbytes_tta);
		t.d = fseeko(infile, -t.o, SEEK_CUR);
		if ( t.d != 0 ){
			error_sys(
				errno, "fseeko", strerror(errno), infile_name
			);
		}
	}

	// adjust readlen and ni32_target
	if ( user->nbytes_tta_total + *readlen > framesize_tta ){
		*readlen = framesize_tta - user->nbytes_tta_total;
	}
	if ( user->ni32_total + *ni32_target > user->ni32_perframe ){
		*ni32_target = user->ni32_perframe - user->ni32_total;
	}

	return;
}

//--------------------------------------------------------------------------//

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

// EOF ///////////////////////////////////////////////////////////////////////
