//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// tta2dec_st.c                                                             //
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

#include "../bits.h"
#include "../libttaR.h"
#include "../splint.h"

#include "bufs.h"
#include "cli.h"
#include "debug.h"
#include "formats.h"
#include "main.h"
#include "tta2.h"

//////////////////////////////////////////////////////////////////////////////

#undef priv
#undef user
#undef decbuf
#undef outfile
#undef infile
static uint ttadec_frame_st(
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	const struct DecBuf *const restrict decbuf,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const, size_t, size_t,
	enum TTASampleBytes, uint, size_t
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*priv,
		*user_out,
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
ttadec_frame_adjust_st(
	size_t *const restrict readlen, size_t *const restrict ni32_target,
	const struct LibTTAr_CodecState_User *const restrict,
	FILE *const restrict infile, const char *const, size_t, size_t
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*readlen,
		*ni32_target,
		infile
@*/
;

//////////////////////////////////////////////////////////////////////////////

void
ttadec_loop_st(
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct DecStats *const restrict dstat_out,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*seektable,
		*dstat_out,
		outfile,
		infile
@*/
{
	const uint                nchan       = (uint) fstat->nchan;
	const enum TTASampleBytes samplebytes = fstat->samplebytes;

	struct LibTTAr_CodecState_Priv *priv = NULL;
	struct LibTTAr_CodecState_User user;
	struct DecBuf decbuf;
	struct DecStats dstat;
	//
	size_t ni32_perframe, framesize_tta;
	uint dec_retval;
	u32 crc_read;
	union {	size_t	z;
		int	d;
	} t;

	memset(&dstat, 0x00, sizeof dstat);

	// setup buffers
	t.z = decbuf_init(&decbuf, g_samplebuf_len, nchan, samplebytes);
	assert(t.z == (size_t) (g_samplebuf_len * nchan));
	//
	t.z = libttaR_codecstate_priv_size(nchan);
	assert(t.z != 0);
	priv = malloc(t.z);
	if UNLIKELY ( priv == NULL ){
		error_sys(errno, "malloc", NULL);
	}
	assert(priv != NULL);

	ni32_perframe = fstat->buflen;
	do {
		if ( (! g_flag.quiet) && (dstat.nframes % SPINNER_FRQ == 0) ){
			errprint_spinner();
		}

		// calc framelen and adjust if necessary
		t.z = dstat.nsamples_perchan + fstat->framelen;
		if ( t.z > fstat->nsamples ){
			t.z = fstat->nsamples - dstat.nsamples_perchan;
			ni32_perframe = t.z * nchan;
		}
		//
		framesize_tta  = letoh32(seektable->table[dstat.nframes]);
		framesize_tta -= (sizeof crc_read);

		// decode and write frame
		dec_retval = ttadec_frame_st(
			priv, &user, &decbuf, outfile, outfile_name, infile,
			infile_name, dstat.nframes, framesize_tta,
			samplebytes, nchan, ni32_perframe
		);

		// check frame crc
		t.z = fread(&crc_read, (size_t) 1, sizeof crc_read, infile);
		if UNLIKELY ( t.z != sizeof crc_read ){
			if UNLIKELY ( ferror(infile) != 0 ){
				error_sys(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: truncated file /"
					" malformed seektable", infile_name
				);
				// TODO retry decoding without seektable
			}
		}
		user.nbytes_tta_total += sizeof crc_read;
		if UNLIKELY ( user.crc != letoh32(crc_read) ){
			warning_tta("%s: frame %zu is corrupted; bad crc",
				infile_name, dstat.nframes
			);
		}

		dstat.nframes		+= (size_t) 1;
		dstat.nsamples		+= user.ni32_total;
		dstat.nsamples_perchan	+= (size_t) (user.ni32_total / nchan);
		dstat.nbytes_decoded	+= user.nbytes_tta_total;
	}
	while ( (user.ni32_total == fstat->buflen) && (dec_retval == 0) );

	// cleanup
	free(priv);
	decbuf_free(&decbuf);

	*dstat_out = dstat;
	return;
}

//--------------------------------------------------------------------------//

// returns 0 on OK, or number of bytes padded
static uint
ttadec_frame_st(
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user_out,
	const struct DecBuf *const restrict decbuf,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name,
	size_t frame_num, size_t framesize_tta,
	enum TTASampleBytes samplebytes, uint nchan, size_t ni32_perframe
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*priv,
		*user_out,
		*decbuf->i32buf,
		*decbuf->ttabuf,
		outfile,
		infile
@*/
{
	uint r = 0;
	struct LibTTAr_CodecState_User user = LIBTTAr_CODECSTATE_USER_INIT;
	size_t readlen, nbytes_read, ni32_target;
	union {	size_t	z;
		uint	u;
		int	d;
	} t;

	ni32_target = (decbuf->i32buf_len < ni32_perframe
		? decbuf->i32buf_len
		: ni32_perframe
	);
	readlen = (decbuf->ttabuf_len < framesize_tta
		? decbuf->ttabuf_len
		: framesize_tta
	);
	goto loop_entr;
	do {
		// adjust for next chunk
		ttadec_frame_adjust_st(
			&readlen, &ni32_target, &user, infile, infile_name,
			framesize_tta, ni32_perframe
		);
loop_entr:
		// read tta from infile
		nbytes_read = fread(
			decbuf->ttabuf, (size_t) 1, readlen, infile
		);
		if UNLIKELY ( nbytes_read != readlen ){
			if UNLIKELY ( feof(infile) != 0 ){
				error_sys_nf(errno, "fread", infile_name);
			}
			else {	warning_tta("%s: frame %zu: truncated file",
					infile_name, frame_num
				);
			}
		}

		// decode tta to i32
		t.d = libttaR_tta_decode(
			decbuf->i32buf, decbuf->ttabuf, decbuf->i32buf_len,
			decbuf->ttabuf_len, ni32_target, nbytes_read, priv,
			&user, samplebytes, nchan, ni32_perframe
		);
		assert(t.d == 0);

		// check for truncated sample
		if ( user.ncalls_codec != 0 ){
			t.u = (uint) (user.ni32_total % nchan);
			if UNLIKELY ( t.u != 0 ){
				warning_tta("%s: frame %zu: "
					"last sample truncated, zero-padding",
					infile_name, frame_num
				);
				r = dec_frame_zeropad(
					decbuf->i32buf, user.ni32, t.u, nchan
				);
				ni32_perframe   += r;
				user.ni32       += r;
				user.ni32_total += r;
			}
		}

		// convert i32 to pcm
		t.z = libttaR_pcm_write(
			decbuf->pcmbuf, decbuf->i32buf, user.ni32,
			samplebytes
		);
		assert(t.z == user.ni32);

		// write pcm to outfile
		t.z = fwrite(
			decbuf->pcmbuf, samplebytes, user.ni32, outfile
		);
		if UNLIKELY ( t.z != user.ni32 ){
			error_sys_nf(errno, "fwrite", outfile_name);
		}
	}
	while ( (user.ncalls_codec != 0) && (r == 0) );

	*user_out = user;
	return r;
}

// MAYBE inline
static void
ttadec_frame_adjust_st(
	size_t *const restrict readlen, size_t *const restrict ni32_target,
	const struct LibTTAr_CodecState_User *const restrict user,
	FILE *const restrict infile, const char *const infile_name,
	size_t framesize_tta, size_t ni32_perframe
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		*readlen,
		*ni32_target,
		infile
@*/
{
	union {	off_t	o;
		int	d;
	} t;

	// seek infile to first non-decoded byte
	if ( user->nbytes_tta < *readlen ){
		t.o  = (off_t) (*readlen - user->nbytes_tta);
		t.d = fseeko(infile, -t.o, SEEK_CUR);
		if UNLIKELY ( t.d != 0 ){
			error_sys(errno, "fseeko", infile_name);
		}
	}

	// adjust readlen and ni32_target
	if ( user->nbytes_tta_total + *readlen > framesize_tta ){
		*readlen = framesize_tta - user->nbytes_tta_total;
	}
	if ( user->ni32_total + *ni32_target > ni32_perframe ){
		*ni32_target = ni32_perframe - user->ni32_total;
	}

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
