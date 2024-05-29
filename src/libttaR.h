#ifndef LIBTTAr_H
#define LIBTTAr_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR.h - 1.1                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* //////////////////////////////////////////////////////////////////////// */

#define LIBTTAr_PURE		__attribute__((pure))
#define LIBTTAr_CONST		__attribute__((const))

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

/* seconds per TTA1 frame */
#define TTA1_FRAME_TIME		((double) 1.04489795918367346939)

#define TTA_CRC32_INIT		((uint32_t) 0xFFFFFFFFu)

/* perchannel safety margin
  >
   the max nbytes_tta that could be read/written for 1 sample
*/
#define TTABUF_SAFETY_MARGIN	((size_t) 1024u)

/* ######################################################################## */

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

/* ######################################################################## */

/* private state for the codec functions; see libttaR_codecstate_priv_size */
struct LibTTAr_CodecState_Priv;

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// struct LibTTAr_CodecState_User                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              user readable state for the codec functions. _must_ be      //
//          initialized before the first call for every new frame           //
//                                                                          //
// fields:                                                                  //
//                                                                          //
//      ncalls_codec:                                                       //
//              number of times the codec function has been called for the  //
//          current frame                                                   //
//                                                                          //
//      crc:                                                                //
//              frame CRC                                                   //
//                                                                          //
//      ni32:                                                               //
//              number of I32 read/written in call                          //
//                                                                          //
//      ni32_total:                                                         //
//              total number of I32 read/written across all calls for the   //
//          current frame                                                   //
//                                                                          //
//      nbytes_tta:                                                         //
//              number of TTA bytes written/read in call                    //
//                                                                          //
//      nbytes_tta_total:                                                   //
//              total number of TTA bytes written/read across all calls for //
//          the current frame                                               //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
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

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_tta_encode                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              a re-entrant TTA encoder that can work on an arbitrary      //
//          number of samples at a time                                     //
//                                                                          //
// return:                                                                  //
//                                                                          //
//      LIBTTAr_RET_(DONE | AGAIN | >=INVAL | MISCONFIG)                    //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      *dest:                                                              //
//              TTA buffer                                                  //
//                                                                          //
//      *src:                                                               //
//              I32 buffer                                                  //
//                                                                          //
//      dest_len:                                                           //
//              size of the TTA buffer                                      //
//                                                                          //
//      src_len:                                                            //
//              nmemb of I32 buffer                                         //
//                                                                          //
//      ni32_target:                                                        //
//              target number of I32 to encode, _must_ be evenly divisible  //
//          by nchan                                                        //
//                                                                          //
//      *priv:                                                              //
//              private state struct                                        //
//                                                                          //
//      *user:                                                              //
//              user state struct                                           //
//                                                                          //
//      samplebytes:                                                        //
//              bytes-per-sample (1u, 2u, 3u)                               //
//                                                                          //
//      nchan:                                                              //
//              number of audio channels                                    //
//                                                                          //
//      ni32_perframe:                                                      //
//              number of I32 in the frame to encode                        //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
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

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_tta_decode                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              a re-entrant TTA decoder that can work on an arbitrary      //
//          number of samples at a time                                     //
//                                                                          //
// return:                                                                  //
//                                                                          //
//      LIBTTAr_RET_(DONE | AGAIN | DECFAIL | >=INVAL | MISCONFIG)          //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      *dest:                                                              //
//              I32 buffer                                                  //
//                                                                          //
//      *src:                                                               //
//              TTA buffer                                                  //
//                                                                          //
//      dest_len:                                                           //
//              nmemb of the I32 buffer                                     //
//                                                                          //
//      src_len:                                                            //
//              size of the TTA buffer                                      //
//                                                                          //
//      ni32_target:                                                        //
//              target number of I32 to write, _must_ be evenly divisible   //
//          by nchan                                                        //
//                                                                          //
//      nbytes_tta_target:                                                  //
//              target number of TTA bytes to decode                        //
//                                                                          //
//      *priv:                                                              //
//              private state struct                                        //
//                                                                          //
//      *user:                                                              //
//              user state struct                                           //
//                                                                          //
//      samplebytes:                                                        //
//              bytes-per-sample (1u, 2u, 3u)                               //
//                                                                          //
//      nchan:                                                              //
//              number of audio channels                                    //
//                                                                          //
//      ni32_perframe:                                                      //
//              number of I32 in the frame to decode                        //
//                                                                          //
//      nbytes_tta_perframe:                                                //
//              number of TTA bytes in the frame to decode                  //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
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
#undef nbytes_tta_total
/*@external@*/ /*@unused@*/
extern int libttaR_tta_decode(
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
	size_t nbytes_tta_total
)
/*@modifies	*dest,
		*priv,
		*user
@*/
;

/* ######################################################################## */

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_codecstate_priv_size                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              calculates the size of the private state struct (for        //
//          passing to an allocator)                                        //
//                                                                          //
// return:                                                                  //
//                                                                          //
//              the size of the struct, or 0 on failure (bad parameter)     //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      nchan:                                                              //
//              number of audio channels                                    //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
#undef nchan
/*@external@*/ /*@unused@*/
extern LIBTTAr_CONST size_t libttaR_codecstate_priv_size(unsigned int nchan)
/*@*/
;

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_ttabuf_safety_margin                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              calculates a safety-margin size for the dest/src buffer for //
//          libttaR_tta_encode/libttaR_tta_decode, respectively. when       //
//          decoding the whole frame at once, the return value is how much  //
//          to pad the src TTA buffer by to ensure that a second function   //
//          call will not be needed.                                        //
//                                                                          //
//              not using this function may result in a >=LIBTTAr_RET_INVAL //
//          return from a codec function if the dest/src _len is too small. //                                                          //
//                                                                          //
// return:                                                                  //
//                                                                          //
//              safety margin for the TTA buffer                            //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      nchan:                                                              //
//              number of audio channels                                    //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
#undef nchan
/*@external@*/ /*@unused@*/
extern LIBTTAr_CONST size_t libttaR_ttabuf_safety_margin(unsigned int nchan)
/*@*/
;

#define libttaR_ttabuf_safety_margin(nchan) ( \
	(size_t) (TTABUF_SAFETY_MARGIN * (nchan)) \
)

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_nsamples_perframe_tta1                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              calculates the number of samples per TTA1 frame             //
//                                                                          //
// return:                                                                  //
//                                                                          //
//              the number of samples per TTA1 frame                        //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      samplerate:                                                         //
//              audio sampling frequency in samples-per-second              //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
#undef samplerate
/*@external@*/ /*@unused@*/
extern LIBTTAr_CONST size_t libttaR_nsamples_perframe_tta1(size_t samplerate)
/*@*/
;

#define libttaR_nsamples_perframe_tta1(samplerate) ( \
	(size_t) (TTA1_FRAME_TIME * (samplerate)) \
)

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_test_nchan                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              tests whether libttaR was configured to support nchan audio //
//          channels                                                        //
//                                                                          //
// return:                                                                  //
//                                                                          //
//              true or false                                               //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      nchan:                                                              //
//              number of audio channels                                    //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
#undef nchan
/*@external@*/ /*@unused@*/
extern LIBTTAr_CONST bool libttaR_test_nchan(unsigned int nchan)
/*@*/
;

/* ######################################################################## */

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_pcm_read                                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              converts u8, i16le, or i24le to I32                         //
//                                                                          //
// return:                                                                  //
//                                                                          //
//              nsamples                                                    //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      *dest:                                                              //
//              I32 buffer                                                  //
//                                                                          //
//      *src:                                                               //
//              PCM buffer                                                  //
//                                                                          //
//      nsamples:                                                           //
//              number of PCM samples                                       //
//                                                                          //
//      samplebytes:                                                        //
//              bytes-per-sample (1u, 2u, 3u)                               //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
#undef dest
#undef src
#undef nsamples
#undef samplebytes
/*@external@*/ /*@unused@*/
extern size_t libttaR_pcm_read(
	int32_t *dest,
	const uint8_t *src,
	size_t nsamples,
	enum TTASampleBytes samplebytes
)
/*@modifies	*dest@*/
;

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_pcm_write                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              converts I32 to u8, i16le, or i24le                         //
//                                                                          //
// return:                                                                  //
//                                                                          //
//              nsamples                                                    //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      *dest:                                                              //
//              PCM buffer                                                  //
//                                                                          //
//      *src:                                                               //
//              I32 buffer                                                  //
//                                                                          //
//      nsamples:                                                           //
//              number of I32 samples                                       //
//                                                                          //
//      samplebytes:                                                        //
//              bytes-per-sample (1u, 2u, 3u)                               //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
#undef dest
#undef src
#undef nsamples
#undef samplebytes
/*@external@*/ /*@unused@*/
extern size_t libttaR_pcm_write(
	uint8_t *dest,
	const int32_t *src,
	size_t nsamples,
	enum TTASampleBytes samplebytes
)
/*@modifies	*dest@*/
;

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_crc32                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              calculates a CRC with the following properties:             //
//                      width   : 32                                        //
//                      endian  : little                                    //
//                      poly    : 0xEDB88320u                               //
//                      xor-in  : 0xFFFFFFFFu                               //
//                      xor-out : 0xFFFFFFFFu                               //
//                                                                          //
// return:                                                                  //
//                                                                          //
//              the CRC                                                     //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      *buf:                                                               //
//              the data to read                                            //
//                                                                          //
//      size:                                                               //
//              size of the data                                            //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
#undef buf
#undef size
/*@external@*/ /*@unused@*/
extern LIBTTAr_PURE uint32_t libttaR_crc32(const uint8_t *buf, size_t size)
/*@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif
