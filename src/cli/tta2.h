#ifndef TTA_TTA2_H
#define TTA_TTA2_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// tta2.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

// tta2dec

#undef buf
#undef user
extern uint ttadec_frame_zeropad(
	i32 *const restrict buf,
	struct LibTTAr_CodecState_User *const restrict user, uint, uint
)
/*@modifies	*buf,
		user->ni32_perframe,
		user->ni32,
		user->ni32_total
@*/
;

//--------------------------------------------------------------------------//

// tta2dec_st

extern void ttadec_loop_st(
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user,
	struct DecBuf *const restrict decbuf,
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct DecStats *const restrict dstat,
	const struct FileStats *const restrict,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*priv,
		*user,
		*decbuf->i32buf,
		*decbuf->ttabuf,
		*seektable,
		*dstat,
		outfile,
		infile
@*/
;

//--------------------------------------------------------------------------//

// tta2enc.c

#undef buf
#undef nmemb_read
#undef ni32_perframe
extern uint ttaenc_frame_zeropad(
	i32 *const restrict buf, size_t *const restrict nmemb_read,
	size_t *const restrict ni32_perframe, uint, uint
)
/*@modifies	*buf,
		*nmemb_read,
		*ni32_perframe
@*/
;

//--------------------------------------------------------------------------//

// tta2enc_st.c

#undef priv
#undef user
#undef encbuf
#undef seektable
#undef estat
#undef outfile
#undef infile
extern void ttaenc_loop_st(
	/*@reldef@*/ struct LibTTAr_CodecState_Priv *const restrict priv,
	/*@out@*/ struct LibTTAr_CodecState_User *const restrict user,
	struct EncBuf *const restrict encbuf,
	struct SeekTable *const restrict seektable,
	/*@out@*/ struct EncStats *const restrict estat,
	const struct FileStats *const restrict,
	FILE *const restrict outfile, const char *const,
	FILE *const restrict infile, const char *const
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
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
