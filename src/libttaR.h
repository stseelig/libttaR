#ifndef LIBTTAr_H
#define LIBTTAr_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR.h - 2.0                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      For library usage information, read the manpages.                   //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

/* //////////////////////////////////////////////////////////////////////// */

#if __STDC_VERSION__ >= 199901L
#define X_LIBTTAr_RESTRICT		restrict
#elif defined(__GNUC__)
#define X_LIBTTAr_RESTRICT		__restrict__
#else
#define X_LIBTTAr_RESTRICT
#endif

/* ------------------------------------------------------------------------ */

#ifdef __GNUC__

#ifdef __has_attribute
#define X_LIBTTAr_HAS_ATTRIBUTE(x)	__has_attribute(x)
#else
#define X_LIBTTAr_HAS_ATTRIBUTE(x)	0
#endif

#if X_LIBTTAr_HAS_ATTRIBUTE(pure)
#define X_LIBTTAr_PURE			__attribute__((pure))
#else
#define X_LIBTTAr_PURE
#endif

#if X_LIBTTAr_HAS_ATTRIBUTE(const)
#define X_LIBTTAr_CONST			__attribute__((const))
#else
#define X_LIBTTAr_CONST
#endif

#else /* ! defined(__GNUC__) */

#define X_LIBTTAr_PURE
#define X_LIBTTAr_CONST

#endif /* __GNUC__ */

/* //////////////////////////////////////////////////////////////////////// */

#define X_LIBTTAr_RV_DONE		 0
#define X_LIBTTAr_RV_AGAIN		 1
#define X_LIBTTAr_RV_FAIL		 2
#define X_LIBTTAr_RV_INVAL_RANGE	-1
#define X_LIBTTAr_RV_INVAL_TRUNC	-2
#define X_LIBTTAr_RV_INVAL_BOUNDS	-3
#define X_LIBTTAr_RV_MISCONFIG		SCHAR_MIN

enum LibTTAr_EncRetVal {
	LIBTTAr_ERV_DONE		= X_LIBTTAr_RV_DONE,
	LIBTTAr_ERV_AGAIN		= X_LIBTTAr_RV_AGAIN,
	LIBTTAr_ERV_INVAL_RANGE		= X_LIBTTAr_RV_INVAL_RANGE,
	LIBTTAr_ERV_INVAL_TRUNC		= X_LIBTTAr_RV_INVAL_TRUNC,
	LIBTTAr_ERV_INVAL_BOUNDS	= X_LIBTTAr_RV_INVAL_BOUNDS,
	LIBTTAr_ERV_MISCONFIG		= X_LIBTTAr_RV_MISCONFIG
};

enum LibTTAr_DecRetVal {
	LIBTTAr_DRV_DONE		= X_LIBTTAr_RV_DONE,
	LIBTTAr_DRV_AGAIN		= X_LIBTTAr_RV_AGAIN,
	LIBTTAr_DRV_FAIL		= X_LIBTTAr_RV_FAIL,
	LIBTTAr_DRV_INVAL_RANGE		= X_LIBTTAr_RV_INVAL_RANGE,
	LIBTTAr_DRV_INVAL_TRUNC		= X_LIBTTAr_RV_INVAL_TRUNC,
	LIBTTAr_DRV_INVAL_BOUNDS	= X_LIBTTAr_RV_INVAL_BOUNDS,
	LIBTTAr_DRV_MISCONFIG		= X_LIBTTAr_RV_MISCONFIG
};

/* ------------------------------------------------------------------------ */

enum LibTTAr_SampleBytes {
	LIBTTAr_SAMPLEBYTES_1		= 1u,
	LIBTTAr_SAMPLEBYTES_2		= 2u,
	LIBTTAr_SAMPLEBYTES_3		= 3u
};
#define LIBTTAr_SAMPLEBYTES_MAX		  3u
#define LIBTTAr_SAMPLEBITS_MAX		 24u

/* //////////////////////////////////////////////////////////////////////// */

struct LibTTAr_CodecState_Priv;

/* ------------------------------------------------------------------------ */

struct LibTTAr_CodecState_User {
	uint32_t	ncalls_codec;
	uint32_t	crc;
	size_t		ni32;
	size_t		ni32_total;
	size_t		nbytes_tta;
	size_t		nbytes_tta_total;
};

#define LIBTTAr_CODECSTATE_USER_INIT	{0, UINT32_MAX, 0, 0, 0, 0}

/* ------------------------------------------------------------------------ */

struct LibTTAr_EncMisc {
	size_t				dest_len;
	size_t				src_len;
	size_t				ni32_target;
	size_t				ni32_perframe;
	enum LibTTAr_SampleBytes	samplebytes;
	unsigned int			nchan;
};

struct LibTTAr_DecMisc {
	size_t				dest_len;
	size_t				src_len;
	size_t				ni32_target;
	size_t				nbytes_tta_target;
	size_t				ni32_perframe;
	size_t				nbytes_tta_perframe;
	enum LibTTAr_SampleBytes	samplebytes;
	unsigned int			nchan;
};

/* //////////////////////////////////////////////////////////////////////// */

struct LibTTAr_VersionInfo {
	unsigned int	 version;
	unsigned int	 version_major;
	unsigned int 	 version_minor;
	unsigned int	 version_revis;
	/*@observer@*/
	const char	*version_extra;
	/*@observer@*/
	const char	*version_date;
	/*@observer@*/
	const char	*copyright;
	/*@observer@*/
	const char	*license;
};

/*@unchecked@*/ /*@unused@*/
extern const struct LibTTAr_VersionInfo libttaR_info;

/* //////////////////////////////////////////////////////////////////////// */

#undef dest
#undef src
#undef priv
#undef user
#undef misc
/*@external@*/ /*@unused@*/
extern enum LibTTAr_EncRetVal libttaR_tta_encode(
	/*@reldef@*/
	uint8_t *X_LIBTTAr_RESTRICT dest,
	/*@in@*/
	const int32_t *X_LIBTTAr_RESTRICT src,
	/*@reldef@*/
	struct LibTTAr_CodecState_Priv *X_LIBTTAr_RESTRICT priv,
	/*@in@*/
	struct LibTTAr_CodecState_User *X_LIBTTAr_RESTRICT user,
	/*@in@*/
	const struct LibTTAr_EncMisc *X_LIBTTAr_RESTRICT misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;

#undef dest
#undef src
#undef priv
#undef user
#undef misc
/*@external@*/ /*@unused@*/
extern enum LibTTAr_DecRetVal libttaR_tta_decode(
	/*@reldef@*/
	int32_t *X_LIBTTAr_RESTRICT dest,
	/*@in@*/
	const uint8_t *X_LIBTTAr_RESTRICT src,
	/*@reldef@*/
	struct LibTTAr_CodecState_Priv *X_LIBTTAr_RESTRICT priv,
	/*@in@*/
	struct LibTTAr_CodecState_User *X_LIBTTAr_RESTRICT user,
	/*@in@*/
	const struct LibTTAr_DecMisc *X_LIBTTAr_RESTRICT misc
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;

/* ------------------------------------------------------------------------ */

#undef dest
#undef src
#undef nsamples
#undef samplebytes
/*@external@*/ /*@unused@*/
extern size_t libttaR_pcm_read(
	/*@out@*/
	int32_t *X_LIBTTAr_RESTRICT dest,
	/*@in@*/
	const uint8_t *X_LIBTTAr_RESTRICT src,
	size_t nsamples,
	enum LibTTAr_SampleBytes samplebytes
)
/*@modifies	*dest@*/
;

#undef dest
#undef src
#undef nsamples
#undef samplebytes
/*@external@*/ /*@unused@*/
extern size_t libttaR_pcm_write(
	/*@out@*/
	uint8_t *X_LIBTTAr_RESTRICT dest,
	/*@in@*/
	const int32_t *X_LIBTTAr_RESTRICT src,
	size_t nsamples,
	enum LibTTAr_SampleBytes samplebytes
)
/*@modifies	*dest@*/
;

/* ------------------------------------------------------------------------ */

#undef nchan
/*@external@*/ /*@unused@*/
X_LIBTTAr_CONST
extern int libttaR_test_nchan(unsigned int nchan)
/*@*/
;

#undef samplerate
/*@external@*/ /*@unused@*/
X_LIBTTAr_CONST
extern size_t libttaR_nsamples_perframe_tta1(size_t samplerate)
/*@*/
;

#undef samplebytes
#undef nchan
/*@external@*/ /*@unused@*/
X_LIBTTAr_CONST
extern size_t libttaR_ttabuf_safety_margin(
	enum LibTTAr_SampleBytes samplebytes, unsigned int nchan
)
/*@*/
;

#undef nchan
/*@external@*/ /*@unused@*/
X_LIBTTAr_CONST
extern size_t libttaR_codecstate_priv_size(unsigned int nchan)
/*@*/
;

#undef buf
#undef size
/*@external@*/ /*@unused@*/
X_LIBTTAr_PURE
extern uint32_t libttaR_crc32(
	/*@in@*/ const uint8_t *X_LIBTTAr_RESTRICT buf, size_t size
)
/*@*/
;

/* //////////////////////////////////////////////////////////////////////// */

#undef X_LIBTTAr_RESTRICT
#undef X_LIBTTAr_HAS_ATTRIBUTE
#undef X_LIBTTAr_PURE
#undef X_LIBTTAr_CONST

#undef X_LIBTTAr_RV_DONE
#undef X_LIBTTAr_RV_AGAIN
#undef X_LIBTTAr_RV_FAIL
#undef X_LIBTTAr_RV_INVAL_RANGE
#undef X_LIBTTAr_RV_INVAL_TRUNC
#undef X_LIBTTAr_RV_INVAL_BOUNDS
#undef X_LIBTTAr_RV_MISCONFIG

/* EOF //////////////////////////////////////////////////////////////////// */
#endif /* LIBTTAr_H */
