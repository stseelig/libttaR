//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// tta2enc_st.c                                                             //
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
#include "cli.h"	// errprint_spinner
#include "debug.h"
#include "formats.h"
#include "opts.h"	// g_flag.quiet
#include "tta2.h"

//////////////////////////////////////////////////////////////////////////////

#undef priv
#undef user
#undef encbuf
#undef outfile
#undef infile
static uint ttaenc_frame_st(
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@partial@*/ struct LibTTAr_CodecState_User *const restrict user,
	const struct EncBuf *const restrict encbuf,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const, size_t,
	enum TTASampleBytes, uint
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

#undef readlen
#undef infile
static void ttaenc_frame_adjust_st(
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

//////////////////////////////////////////////////////////////////////////////

void
ttaenc_loop_st(
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user,
	struct EncBuf *const restrict encbuf,
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct EncStats *const restrict estat,
	const struct FileStats *const restrict fstat,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*priv,
		*user,
		*encbuf->i32buf,
		*encbuf->ttabuf,
		*seektable,
		*estat,
		outfile,
		infile
@*/
{
	uint enc_retval;
	union {
		size_t z;
	} t;

	// any warning/error in loop gets UNLIKELY'd
	user->ni32_perframe = fstat->buflen;
	memset(estat, 0x00, sizeof *estat);
	do {
		if ( (!g_flag.quiet) && (estat->nframes % (size_t) 64 == 0) ){
			errprint_spinner();
		}

		// calc framelen and adjust if necessary
		t.z  = (estat->nsamples + fstat->buflen);
		t.z *= fstat->samplebytes;
		if ( t.z > fstat->decpcm_size ){
			t.z  = fstat->decpcm_size;
			t.z -= estat->nsamples * fstat->samplebytes;
			t.z /= fstat->samplebytes;
			user->ni32_perframe = t.z;
		}

		// encode and write frame
		enc_retval = ttaenc_frame_st(
			priv, user, encbuf, outfile, outfile_name, infile,
			infile_name, estat->nframes, fstat->samplebytes,
			(uint) fstat->nchan
		);

		// write frame footer (crc)
		user->crc = htole32(user->crc);
		t.z = fwrite(
			&user->crc, (size_t) 1, sizeof user->crc, outfile
		);
		if UNLIKELY ( t.z != sizeof user->crc ){
			error_sys_nf(
				errno, "fwrite", strerror(errno), outfile_name
			);
		}
		user->nbytes_tta_total += sizeof user->crc;

		// update seektable
		seektable_add(
			seektable, user->nbytes_tta_total, estat->nframes,
			outfile_name
		);

		estat->nframes		+= (size_t) 1;
		estat->nsamples		+= user->ni32_total;
		estat->nsamples_perchan	+= (size_t) (
			user->ni32_total / fstat->nchan
		);
		estat->nbytes_encoded	+= user->nbytes_tta_total;
	}
	while (	(user->ni32_total == fstat->buflen) && (enc_retval == 0) );

	return;
}

//--------------------------------------------------------------------------//

// returns 0 on OK, or number of bytes padded
static uint
ttaenc_frame_st(
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@partial@*/ struct LibTTAr_CodecState_User *const restrict user,
	const struct EncBuf *const restrict encbuf,
	FILE *const restrict outfile, const char *const outfile_name,
	FILE *const restrict infile, const char *const infile_name,
	size_t frame_num, enum TTASampleBytes samplebytes, uint nchan
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
	t.z = g_samplebuf_len * nchan;
	readlen = (t.z < user->ni32_perframe ? t.z : user->ni32_perframe);
	goto loop_entr;
	do {
		// adjust for next chunk
		ttaenc_frame_adjust_st(
			&readlen, user, infile, infile_name, samplebytes
		);
loop_entr:
		// read pcm from infile
		nmemb_read = fread(
			encbuf->pcmbuf, (size_t) samplebytes, readlen, infile
		);
		if UNLIKELY ( nmemb_read != readlen ){
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
			samplebytes
		);
		assert(t.z == nmemb_read);

		// check for truncated sample
		t.u = (uint) (nmemb_read % nchan);
		if UNLIKELY ( t.u != 0 ){
			warning_tta("%s: frame %zu: last sample truncated, "
				"zero-padding", infile_name, frame_num
			);
			r = ttaenc_frame_zeropad(
				encbuf->i32buf, &nmemb_read,
				&user->ni32_perframe, t.u, nchan
			);
		}

		// encode i32 to tta
		t.d = libttaR_tta_encode(
			encbuf->ttabuf, encbuf->i32buf, encbuf->ttabuf_len,
			encbuf->i32buf_len, nmemb_read, priv, user,
			samplebytes, nchan
		);
		assert(t.d == 0);

		// write tta to outfile
		t.z = fwrite(
			encbuf->ttabuf, (size_t) 1, user->nbytes_tta, outfile
		);
		// user->nbytes_tta could be 0 for a small g_samplebuf_len
		if UNLIKELY ( t.z != user->nbytes_tta ){
			error_sys_nf(
				errno, "fwrite", strerror(errno), outfile_name
			);
		}
	}
	while ( (! user->frame_is_finished) && (r == 0) );

	return r;
}

static void
ttaenc_frame_adjust_st(
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
		if UNLIKELY ( t.d != 0 ){
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

// EOF ///////////////////////////////////////////////////////////////////////
