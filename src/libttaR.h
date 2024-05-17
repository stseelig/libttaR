#ifndef LIBTTAr_H
#define LIBTTAr_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR.h - 1.1.0                                                        //
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

/* ######################################################################## */

enum LibTTAr_Ret {
	/* frame was finished without error*/
	LIBTTAr_RET_DONE	 =  0,

	/* frame was not finished */
	LIBTTAr_RET_AGAIN	 =  1,

	/* frame was finished, but nbytes_tta_total != nbytes_tta_perframe */
	LIBTTAr_RET_DECFAIL      =  2,

	/* bad dest_len, src_len, ni32_target, nbytes_tta_target,
	     ni32_perframe, or nbytes_tta_perframe */
	LIBTTAr_RET_INVAL_BOUNDS,

	/* bad samplebytes or nchan */
	LIBTTAr_RET_INVAL_DIMEN,

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

/* seconds per TTA frame */
#define TTA_FRAME_TIME		((double) 1.04489795918367346939)

#define TTA_CRC32_INIT		((uint32_t) 0xFFFFFFFFu)

/* ######################################################################## */

/* private state for the codec functions */
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
//          initialized before the first call to code a frame               //
//                                                                          //
// fields:                                                                  //
//                                                                          //
//      ncalls_codec:                                                       //
//              number of times the codec function has been called          //
//                                                                          //
//      crc:                                                                //
//              frame CRC                                                   //
//                                                                          //
//      ni32:                                                               //
//              number of I32 read/written in call                          //
//                                                                          //
//      ni32_total:                                                         //
//              total number of I32 read/written across all calls for the   //
//          same frame                                                      //
//                                                                          //
//      nbytes_tta:                                                         //
//              number of TTA bytes written/read in call                    //
//                                                                          //
//      nbytes_tta_total:                                                   //
//              total number of TTA bytes written/read across all calls for //
//          the same frame                                                  //
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

/* version numbers */
/*@unchecked@*/ /*@unused@*/
extern const unsigned int libttaR_num_version;
/*@unchecked@*/ /*@unused@*/
extern const unsigned int libttaR_num_version_major;
/*@unchecked@*/ /*@unused@*/
extern const unsigned int libttaR_num_version_minor;
/*@unchecked@*/ /*@unused@*/
extern const unsigned int libttaR_num_version_revis;

/* version strings */
/*@observer@*/ /*@unchecked@*/ /*@unused@*/
extern const char *const libttaR_str_version_extra;
/*@observer@*/ /*@unchecked@*/ /*@unused@*/
extern const char *const libttaR_str_version_date;

/* copyright string */
/*@observer@*/ /*@unchecked@*/ /*@unused@*/
extern const char *const libttaR_str_copyright;

/* license string */
/*@observer@*/ /*@unchecked@*/ /*@unused@*/
extern const char *const libttaR_str_license;

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
//          number of samples-per-channel at a time                         //
//                                                                          //
// return:                                                                  //
//                                                                          //
//      LIBTTAr_RET_DONE (0)                                                //
//      LIBTTAr_RET_AGAIN                                                   //
//      LIBTTAr_RET_INVAL*                                                  //
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
//              number of I32 per frame                                     //
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
//          number of samples-per-channel at a time                         //
//                                                                          //
// return:                                                                  //
//                                                                          //
//      LIBTTAr_RET_DONE (0)                                                //
//      LIBTTAr_RET_AGAIN                                                   //
//      LIBTTAr_RET_DECFAIL                                                 //
//      LIBTTAr_RET_INVAL*                                                  //
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
//              number of I32 per frame                                     //
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
extern size_t libttaR_codecstate_priv_size(unsigned int nchan)
/*@*/
;

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_ttabuf_size                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              calculates a buffer size for the dest/src buffer for        //
//          tta_encode/tta_decode, respectively                             //
//                                                                          //
// return:                                                                  //
//                                                                          //
//              size for a TTA buffer that is safe, or 0 on failure (bad    //
//          parameters)                                                     //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      nsamples:                                                           //
//              target number of PCM samples-per-channel to code            //
//                                                                          //
//      nchan:                                                              //
//              number of audio channels                                    //
//                                                                          //
//      samplesbytes:                                                       //
//              bytes-per-sample (1u, 2u, 3u)                               //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
#undef nsamples
#undef nchan
#undef samplebytes
/*@external@*/ /*@unused@*/
extern size_t libttaR_ttabuf_size(
	size_t nsamples,
	unsigned int nchan,
	enum TTASampleBytes samplebytes
)
/*@*/
;

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
extern bool libttaR_test_nchan(unsigned int nchan)
/*@*/
;

/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_nsamples_perframe                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//                                                                          //
//              calculates the number of samples-per-channel per TTA frame  //
//                                                                          //
// return:                                                                  //
//                                                                          //
//              the number of samples-per-channel per TTA frame             //
//                                                                          //
// parameters:                                                              //
//                                                                          //
//      samplerate:                                                         //
//              audio sampling frequency in samples-per-second              //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */
#undef samplerate
/*@external@*/ /*@unused@*/
extern size_t libttaR_nsamples_perframe(size_t samplerate)
/*@*/
;

#define libttaR_nsamples_perframe(samplerate) ( \
	(size_t) (TTA_FRAME_TIME * samplerate) \
)

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
extern uint32_t libttaR_crc32(const uint8_t *buf, size_t size)
/*@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif
