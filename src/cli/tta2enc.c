//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// tta2enc.c                                                                //
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

static void tta2enc_loop(const struct OpenedFilesMember *const restrict)
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
#undef encbuf
#undef outfile
#undef infile
static uint ttaenc_frame(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@partial@*/ struct LibTTAr_CodecState_User *const restrict user,
	const struct EncBuf *const restrict encbuf,
	const struct FileStats *const restrict,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const, size_t
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*priv,
		*user,
		*encbuf->i32buf,
		*encbuf->ttabuf,
		outfile,
		infile
@*/
;

#undef buf
#undef nmemb_read
#undef ni32_perframe
static uint ttaenc_frame_zeropad(
	i32 *const restrict buf, size_t *const restrict nmemb_read,
	size_t *const restrict ni32_perframe, uint, uint
)
/*@modifies	*buf,
		*nmemb_read,
		*ni32_perframe
@*/
;

#undef readlen
#undef infile
static void ttaenc_frame_adjust(
	size_t *const restrict readlen,
	const struct LibTTAr_CodecState_User *const restrict,
	FILE *const restrict infile, const char *const, enum TTASampleBytes
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*readlen,
		infile
@*/
;

#undef fstat
#undef file
static enum FileCheck filecheck_decfmt(
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
tta2enc(uint optind)
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

	(void) memset(&openedfiles, 0x00, sizeof openedfiles);

	(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);

	// process opts/args
	nerrors_file = optargs_process(
		&openedfiles, optind, tta2enc_optdict
	);

	// program intro
	if ( ! g_flag.quiet ){
		errprint_program_intro();
	}

	// get file stats
	for ( i = 0; i < openedfiles.nmemb; ++i ){

		ofm = openedfiles.file[i];
		if ( ofm->infile == NULL ){ continue; }	// bad filename

		// check for supported filetypes and fill most of fstat
		if ( ! g_flag.rawpcm ){
			t.b = (bool) filecheck_decfmt(
				&ofm->fstat, ofm->infile, ofm->infile_name
			);
			if ( t.b ){
				++nerrors_file;
				continue;
			}
		}
		else {	rawpcm_statcopy(&ofm->fstat);
			// TODO mode for stdin reading
			t.d = fseeko(ofm->infile, 0, SEEK_END);
			if ( t.d != 0 ){
				error_sys(
					errno, "fseeko", ofm->infile_name,
					strerror(errno)
				);
			}
			ofm->fstat.decpcm_off  = 0;
			ofm->fstat.decpcm_size = (size_t) ftello(ofm->infile);
		}

		if ( ! libttaR_test_nchan((uint)ofm->fstat.nchan) ){
			++nerrors_file;
			error_tta_nf("%s: libttaR built without support for"
				" nchan == %u", ofm->infile_name,
				ofm->fstat.nchan
			);
		}

		// the rest of fstat
		ofm->fstat.encfmt      = FORMAT_TTA1;
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

	// encode each file
	for ( i = 0; i < openedfiles.nmemb; ++i ){
		if ( (i != 0) && (! g_flag.quiet) ){
			(void) fputc('\n', stderr);
		}
		tta2enc_loop(openedfiles.file[i]);
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
			MODE_ENCODE
		);
	}

#ifndef NDEBUG
	// cleanup
	openedfiles_close_free(&openedfiles);
#endif
	return (int) g_nwarnings;
}

static void
tta2enc_loop(const struct OpenedFilesMember *const restrict ofm)
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
	const char *infile_name = ofm->infile_name;
	const struct FileStats *const restrict fstat = &ofm->fstat;
	//
	FILE *restrict outfile = NULL;
	char *outfile_name = NULL;
	const char *outfile_dir  = NULL;
	const char *outfile_sfx  = NULL;
	//
	struct LibTTAr_CodecState_Priv *state_priv;
	struct LibTTAr_CodecState_User state_user;
	//
	struct EncStats estat;
	struct SeekTable seektable;
	struct EncBuf encbuf;
	struct timespec ts_start, ts_stop;
	uint enc_retval;
	union {
		size_t	z;
		int	d;
	} t;

	// set outfile_name
	if ( g_flag.outfile != NULL ){
		if ( g_flag.outfile_is_dir ){
			outfile_dir  = g_flag.outfile;
			outfile_name = (char *) infile_name;
			outfile_sfx  = ".tta";
		}
		else {	outfile_name = (char *) g_flag.outfile;
		}
	}
	else {	outfile_name = (char *) infile_name;
		outfile_sfx  = ".tta";
	}
	outfile_name = outfile_name_fmt(
		outfile_dir, outfile_name, outfile_sfx
	);
	g_rm_on_sigint = outfile_name;

	// pre-encode stats
	if ( ! g_flag.quiet ){
		errprint_stats_precodec(
			fstat, infile_name, outfile_name, MODE_ENCODE
		);
	}

	// setup seektable
	switch ( fstat->encfmt ){
	case FORMAT_TTA1:
		// seektable at start of file, size calculated in advance
		t.z  = (fstat->decpcm_size + fstat->buflen) / fstat->buflen;
		--t.z;
		t.z  = (size_t) (t.z + fstat->samplebytes);
		t.z /= fstat->samplebytes;
		seektable_init( &seektable, t.z);
		break;
	}
	// TODO handle file of unknown size
	//else {	// TODO setup tmpfile for writing
	//	seektable_init(&seektable, 0);
	//}

	// open outfile
	outfile = fopen_check(outfile_name, "w", FATAL);
	if ( outfile == NULL ){
		error_sys(errno, "fopen", strerror(errno), outfile_name);
	}
	assert(outfile != NULL);

	// save some space for the outfile header and seektable
	// TODO handle file of unknown size
	switch ( fstat->encfmt ){
	case FORMAT_TTA1:
		prewrite_tta1_header_seektable(outfile, &seektable);
		break;
	}

	// seek to start of pcm
	(void) fseeko(infile, fstat->decpcm_off, SEEK_SET);

	// setup buffers
	t.z = encbuf_init(
		&encbuf, g_samplebuf_len, (uint) fstat->nchan,
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
	//
	(void) memset(&state_user, 0x00, sizeof state_user);
	state_user.ni32_perframe = fstat->buflen;

	if ( ! g_flag.quiet ){
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_start);
	}

	// encode loop
	(void) memset(&estat, 0x00, sizeof estat);
	do {
		if ( (! g_flag.quiet) && (estat.nframes % (size_t) 64 == 0) ){
			errprint_spinner();
		}

		// calc framelen and adjust if necessary
		t.z  = (estat.nsamples + fstat->buflen);
		t.z *= fstat->samplebytes;
		if ( t.z > fstat->decpcm_size ){
			t.z  = fstat->decpcm_size;
			t.z -= estat.nsamples * fstat->samplebytes;
			t.z /= fstat->samplebytes;
			state_user.ni32_perframe = t.z;
		}

		// encode and write frame
		enc_retval = ttaenc_frame(
			state_priv, &state_user, &encbuf, fstat, outfile,
			outfile_name, infile, infile_name, estat.nframes
		);

		// write frame footer (crc)
		state_user.crc = htole32(state_user.crc);
		t.z = fwrite(
			&state_user.crc, (size_t) 1, sizeof state_user.crc,
			outfile
		);
		if ( t.z != sizeof state_user.crc ){
			error_sys_nf(
				errno, "fwrite", strerror(errno), outfile_name
			);
		}
		state_user.nbytes_tta_total += sizeof state_user.crc;

		// update seektable
		seektable_add(
			&seektable, state_user.nbytes_tta_total,
			estat.nframes, outfile_name
		);

		estat.nframes		+= (size_t) 1;
		estat.nsamples		+= state_user.ni32_total;
		estat.nsamples_perchan	+= (size_t) (
			state_user.ni32_total / fstat->nchan
		);
		estat.nbytes_encoded	+= state_user.nbytes_tta_total;
	}
	while ( (state_user.ni32_total == fstat->buflen)
	       &&
	        (enc_retval == 0)
	);

	if ( ! g_flag.quiet ){
		(void) clock_gettime(CLOCK_MONOTONIC, &ts_stop);
		estat.encodetime += timediff(&ts_start, &ts_stop);
	}

	// write header and seektable
	switch ( fstat->encfmt ){
	case FORMAT_TTA1:
		rewind(outfile);
		seektable.off = write_tta1_header(
			outfile, estat.nsamples_perchan, fstat, outfile_name
		);
		write_tta_seektable(outfile, &seektable);
		break;
	}

	// post-encode stats
	if ( ! g_flag.quiet ){
		errprint_stats_postcodec(fstat, &estat);
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
	encbuf_free(&encbuf);
	seektable_free(&seektable);

	return;
}

// returns 0 on OK, or number of bytes padded
static uint
ttaenc_frame(
	/*@out@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@partial@*/ struct LibTTAr_CodecState_User *const restrict user,
	const struct EncBuf *const restrict encbuf,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name,
	size_t frame_num
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*priv,
		*user,
		*encbuf->i32buf,
		*encbuf->ttabuf,
		outfile,
		infile
@*/
{
	uint r = 0;
	size_t readlen, nmemb_read;
	union {
		size_t	z;
		uint	u;
		int	d;
	} t;

	user->is_new_frame = true;
	t.z = g_samplebuf_len * fstat->nchan;
	readlen = (t.z < user->ni32_perframe ? t.z : user->ni32_perframe);
	do {
		// read pcm from infile
		nmemb_read = fread(
			encbuf->pcmbuf, (size_t) fstat->samplebytes, readlen,
			infile
		);
		if ( nmemb_read != readlen ){
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

		// convert pcm to i32
		t.z = libttaR_pcm_read(
			encbuf->i32buf, encbuf->pcmbuf, nmemb_read,
			fstat->samplebytes
		);
		assert(t.z == nmemb_read);

		// check for truncated sample
		t.u = (uint) (nmemb_read % fstat->nchan);
		if ( t.u != 0 ){
			warning_tta("%s: frame %zu: last sample truncated, "
				"zero-padding", infile_name, frame_num
			);
			r = ttaenc_frame_zeropad(
				encbuf->i32buf, &nmemb_read,
				&user->ni32_perframe, t.u, (uint) fstat->nchan
			);
		}

		// encode i32 to tta
		t.d = libttaR_tta_encode(
			encbuf->ttabuf, encbuf->i32buf, encbuf->ttabuf_len,
			encbuf->i32buf_len, nmemb_read, priv, user,
			fstat->samplebytes, (uint) fstat->nchan
		);
		assert(t.d == 0);

		// write tta to outfile
		t.z = fwrite(
			encbuf->ttabuf, (size_t) 1, user->nbytes_tta, outfile
		);
		// user->nbytes_tta could be 0 for a small g_samplebuf_len
		if ( t.z != user->nbytes_tta ){
			error_sys_nf(
				errno, "fwrite", strerror(errno), outfile_name
			);
		}

		if ( r != 0 ){ break; }
		if ( ! user->frame_is_finished ){
			ttaenc_frame_adjust(
				&readlen, user, infile, infile_name,
				fstat->samplebytes
			);
		}
	}
	while ( ! user->frame_is_finished );

	return r;
}

// returns nmemb of buf zero-padded
static uint
ttaenc_frame_zeropad(
	i32 *const restrict buf, size_t *const restrict nmemb_read,
	size_t *const restrict ni32_perframe, uint diff, uint nchan
)
/*@modifies	*buf,
		*nmemb_read,
		*ni32_perframe
@*/
{
	const size_t r = (size_t) (nchan - diff);

	(void) memset(&buf[*nmemb_read], 0x00, r * (sizeof *buf));

	*nmemb_read	+= r;
	*ni32_perframe	+= r;
	return (uint) r;
}

static void
ttaenc_frame_adjust(
	size_t *const restrict readlen,
	const struct LibTTAr_CodecState_User *const restrict user,
	FILE *const restrict infile, const char *const infile_name,
	enum TTASampleBytes samplebytes
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*readlen,
		infile
@*/
{
	union {
		off_t	o;
		int	d;
	} t;

	// seek infile to first non-encoded sample
	if ( user->ni32 < *readlen ){
		t.o  = (off_t) ((*readlen - user->ni32) * samplebytes);
		t.d = fseeko(infile, -t.o, SEEK_CUR);
		if ( t.d != 0 ){
			error_sys(
				errno, "fseeko", strerror(errno), infile_name
			);
		}
	}

	// adjust readlen
	if ( user->ni32_total + *readlen > user->ni32_perframe ){
		*readlen = user->ni32_perframe - user->ni32_total;
	}

	return;
}

//--------------------------------------------------------------------------//

static enum FileCheck
filecheck_decfmt(
	struct FileStats *const restrict fstat, FILE *const restrict file,
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

	// wav
	t.fc = filecheck_wav(fstat, file);
	if ( t.fc == FILECHECK_OK ){ goto end_check; }
	if ( t.fc != FILECHECK_MISMATCH ){
		error_filecheck(t.fc, fstat, filename, errno);
		return t.fc;
	}

	// w64
	t.fc = filecheck_w64(fstat, file);
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
