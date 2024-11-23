#ifndef LIBTTAr_H
#define LIBTTAr_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR.h - 1.2                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      For library usage information, read the manpages.                   //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* //////////////////////////////////////////////////////////////////////// */

#ifdef __GNUC__

#ifdef __has_attribute
#define HAS_ATTRIBUTE(x)	__has_attribute(x)
#else
#define HAS_ATTRIBUTE(x)	0
#endif

#if HAS_ATTRIBUTE(pure)
#define LIBTTAr_PURE		__attribute__((pure))
#else
#define LIBTTAr_PURE
#endif

#if HAS_ATTRIBUTE(const)
#define LIBTTAr_CONST		__attribute__((const))
#else
#define LIBTTAr_CONST
#endif

#else /* ! defined(__GNUC__) */

#define LIBTTAr_PURE
#define LIBTTAr_CONST

#endif /* __GNUC__ */

/* //////////////////////////////////////////////////////////////////////// */

enum LibTTAr_RetVal {
	/* frame finished */
	LIBTTAr_RET_DONE	 =  0,

	/* frame not finished */
	LIBTTAr_RET_AGAIN	 =  1,

	/* frame finished, but (nbytes_tta_total != nbytes_tta_perframe)
	  ||
	   frame not finished, but (nbytes_tta_total > nbytes_tta_perframe)
	*/
	LIBTTAr_RET_DECFAIL      =  2,

	/* (ni32_target % nchan != 0) or other bad parameter
	   used as the base value; functions can return greater values
	*/
	LIBTTAr_RET_INVAL,

	/* library was misconfigured; see libttaR_test_nchan */
	LIBTTAr_RET_MISCONFIG	 = -1
};

enum TTASampleBytes {
	TTASAMPLEBYTES_1 = 1u,
	TTASAMPLEBYTES_2 = 2u,
	TTASAMPLEBYTES_3 = 3u
};
#define TTA_SAMPLEBYTES_MAX	((unsigned int) TTASAMPLEBYTES_3)
#define TTA_SAMPLEBITS_MAX	((unsigned int) (8u*TTA_SAMPLEBYTES_MAX))

#define TTA_CRC32_INIT		((uint32_t) 0xFFFFFFFFu)

/* ######################################################################## */

struct LibTTAr_VersionInfo {
	unsigned int	 version;
	unsigned int	 version_major;	/* API change               */
	unsigned int 	 version_minor;	/* bugfix or improvement    */
	unsigned int	 version_revis;	/* inbetween minor versions */
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

/* ######################################################################## */

/* private state for the codec functions; see libttaR_codecstate_priv_size */
struct LibTTAr_CodecState_Priv;

struct LibTTAr_CodecState_User {
	uint32_t	ncalls_codec;
	uint32_t	crc;
	size_t		ni32;			/* enc: n-read, dec: n-writ */
	size_t		ni32_total;		/* ~                        */
	size_t		nbytes_tta;		/* enc: n-writ, dec: n-read */
	size_t		nbytes_tta_total;	/* ~                        */
};

#define LIBTTAr_CODECSTATE_USER_INIT ((struct LibTTAr_CodecState_User) \
	{0, TTA_CRC32_INIT, 0, 0, 0, 0} \
)

/* ######################################################################## */

#undef dest
#undef src
#undef dest_len
#undef src_len
#undef ni32_target
#undef priv
#undef user
#undef samplebytes
#undef nchan
#undef ni32_perframe
/*@external@*/ /*@unused@*/
extern int libttaR_tta_encode(
	/*@out@*/
	uint8_t *dest,
	const int32_t *src,
	size_t dest_len,
	size_t src_len,
	size_t ni32_target,
	/*@reldef@*/
	struct LibTTAr_CodecState_Priv *priv,
	/*@in@*/
	struct LibTTAr_CodecState_User *user,
	enum TTASampleBytes samplebytes,
	unsigned int nchan,
	size_t ni32_perframe
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;

#undef dest
#undef src
#undef dest_len
#undef src_len
#undef ni32_target
#undef nbytes_tta_target
#undef priv
#undef user
#undef samplebytes
#undef nchan
#undef ni32_perframe
#undef nbytes_tta_perframe
/*@external@*/ /*@unused@*/
extern int libttaR_tta_decode(
	/*@out@*/
	int32_t *dest,
	const uint8_t *src,
	size_t dest_len,
	size_t src_len,
	size_t ni32_target,
	size_t nbytes_tta_target,
	/*@reldef@*/
	struct LibTTAr_CodecState_Priv *priv,
	/*@in@*/
	struct LibTTAr_CodecState_User *user,
	enum TTASampleBytes samplebytes,
	unsigned int nchan,
	size_t ni32_perframe,
	size_t nbytes_tta_perframe
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;

/* ######################################################################## */

#undef nchan
/*@external@*/ /*@unused@*/
extern LIBTTAr_CONST size_t libttaR_codecstate_priv_size(unsigned int nchan)
/*@*/
;

#undef samplebytes
#undef nchan
/*@external@*/ /*@unused@*/
extern LIBTTAr_CONST size_t libttaR_ttabuf_safety_margin(
	enum TTASampleBytes samplebytes, unsigned int nchan
)
/*@*/
;

#undef samplerate
/*@external@*/ /*@unused@*/
extern LIBTTAr_CONST size_t libttaR_nsamples_perframe_tta1(size_t samplerate)
/*@*/
;

#define libttaR_nsamples_perframe_tta1(samplerate) ((size_t) \
	((256u * (samplerate)) / 245u) \
)

#undef nchan
/*@external@*/ /*@unused@*/
extern LIBTTAr_CONST bool libttaR_test_nchan(unsigned int nchan)
/*@*/
;

/* ######################################################################## */

#undef dest
#undef src
#undef nsamples
#undef samplebytes
/*@external@*/ /*@unused@*/
extern size_t libttaR_pcm_read(
	/*@out@*/
	int32_t *dest,
	const uint8_t *src,
	size_t nsamples,
	enum TTASampleBytes samplebytes
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
	uint8_t *dest,
	const int32_t *src,
	size_t nsamples,
	enum TTASampleBytes samplebytes
)
/*@modifies	*dest@*/
;

#undef buf
#undef size
/*@external@*/ /*@unused@*/
extern LIBTTAr_PURE uint32_t libttaR_crc32(const uint8_t *buf, size_t size)
/*@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif /* LIBTTAr_H */
