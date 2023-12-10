#ifndef LIBTTAr_H
#define LIBTTAr_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR.h                                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//##########################################################################//

#ifndef xENUM_TTASAMPLEBYTES
#define xENUM_TTASAMPLEBYTES
enum TTASampleBytes {
	TTASAMPLEBYTES_1 = 1u,
	TTASAMPLEBYTES_2 = 2u,
	TTASAMPLEBYTES_3 = 3u
};
#define TTA_SAMPLEBYTES_MAX	((enum TTASampleBytes) 3u)
#define TTA_SAMPLEBITS_MAX	24u
#endif

// seconds per TTA frame
#define TTA_FRAME_TIME		((double) 1.04489795918367346939)

//##########################################################################//

// version numbers
/*@unchecked@*/
extern const unsigned int libttaR_num_version;
/*@unchecked@*/
extern const unsigned int libttaR_num_version_major;
/*@unchecked@*/
extern const unsigned int libttaR_num_version_minor;
/*@unchecked@*/
extern const unsigned int libttaR_num_version_revis;

// version string
/*@unchecked@*/
extern const char libttaR_str_version[];

// copyright string
/*@unchecked@*/
extern const char libttaR_str_copyright[];

// license string
/*@unchecked@*/
extern const char libttaR_str_license[];

//##########################################################################//

// private state for the codec functions
struct LibTTAr_CodecState_Priv;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// struct LibTTAr_CodecState_User                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              user readable state for the codec functions                 //
//                                                                          //
// user-set fields:                                                         //
//              ni32_perframe:                                              //
//                      number of I32 per frame                             //
//              is_new_frame:                                               //
//                      set to true to start a new frame                    //
// returned fields:                                                         //
//              frame_is_finished:                                          //
//                      codec function will set to true the frame was       //
//      finished                                                            //
//              crc:                                                        //
//                      frame CRC                                           //
//              ni32:                                                       //
//                      number of I32 read/written in call                  //
//              ni32_total:                                                 //
//                      total number of I32 read/written across all calls   //
//      for the same frame                                                  //
//              nbytes_tta:                                                 //
//                      number of TTA bytes written/read in call            //
//              nbytes_tta_total:                                           //
//                      total number of TTA bytes written/read across all   //
//      calls for the same frame                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
struct LibTTAr_CodecState_User {

	// set by user
	size_t		ni32_perframe;		// framelen * nchan
	bool		is_new_frame;

	// set by called function
	bool		frame_is_finished;
	uint32_t	crc;
	size_t		ni32;			// enc: n-read, dec: n-writ
	size_t		ni32_total;		// ~
	size_t		nbytes_tta;		// enc: n-writ, dec: n-read
	size_t		nbytes_tta_total;	// ~
};

//##########################################################################//

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_tta_encode                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              a re-entrant TTA encoder that can work on an arbitrary      //
//      number of samples-per-channel at a time                             //
//                                                                          //
// return:                                                                  //
//              0 on success, non-0 on failure (bad parameters)             //
//                                                                          //
// parameters:                                                              //
//              *dest:                                                      //
//                      TTA buffer                                          //
//              *src:                                                       //
//                      I32 buffer                                          //
//              dest_len:                                                   //
//                      size of the TTA buffer                              //
//              src_len:                                                    //
//                      nmemb of I32 buffer                                 //
//              ni32_target:                                                //
//                      target number of I32 to encode, _must_ be evenly    //
//      divisible by nchan                                                  //
//              *priv:                                                      //
//                      private state struct                                //
//              *user:                                                      //
//                      user state struct                                   //
//              samplebytes:                                                //
//                      bytes-per-sample (1, 2, 3)                          //
//              nchan:                                                      //
//                      number of audio channels                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#undef dest
#undef src
#undef dest_len
#undef src_len
#undef ni32_target
#undef priv
#undef user
#undef samplebytes
#undef nchan
extern int libttaR_tta_encode(
	uint8_t *dest,
	const int32_t *src,
	size_t dest_len,
	size_t src_len,
	size_t ni32_target,
	/*@out@*/
	struct LibTTAr_CodecState_Priv *priv,
	/*@partial@*/
	struct LibTTAr_CodecState_User *user,
	enum TTASampleBytes samplebytes,
	unsigned int nchan
)
/*@modifies	*dest,
		*priv,
		user->is_new_frame,
		user->frame_is_finished,
		user->crc,
		user->ni32,
		user->ni32_total,
		user->nbytes_tta,
		user->nbytes_tta_total
@*/
;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_tta_decode                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              a re-entrant TTA decoder that can work on an arbitrary      //
//      number of samples-per-channel at a time                             //
//                                                                          //
// return:                                                                  //
//              0 on success, non-0 on failure (bad parameters)             //
//                                                                          //
// parameters:                                                              //
//              *dest:                                                      //
//                      I32 buffer                                          //
//              *src:                                                       //
//                      TTA buffer                                          //
//              dest_len:                                                   //
//                      nmemb of the I32 buffer                             //
//              src_len:                                                    //
//                      size of the TTA buffer                              //
//              ni32_target:                                                //
//                      target number of I32 to write, _must_ be evenly     //
//      divisible by nchan                                                  //
//              nbytes_tta_target:                                          //
//                      target number of TTA bytes to decode                //
//              *priv:                                                      //
//                      private state struct                                //
//              *user:                                                      //
//                      user state struct                                   //
//              samplebytes:                                                //
//                      bytes-per-sample (1, 2, 3)                          //
//              nchan:                                                      //
//                      number of audio channels                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#undef dest
#undef src
#undef dest_len
#undef src_len
#undef ni32_target
#undef priv
#undef user
#undef samplebytes
#undef nchan
extern int libttaR_tta_decode(
	int32_t *dest,
	const uint8_t *src,
	size_t dest_len,
	size_t src_len,
	size_t ni32_target,
	size_t nbytes_tta_target,
	/*@out@*/
	struct LibTTAr_CodecState_Priv *priv,
	/*@partial@*/
	struct LibTTAr_CodecState_User *user,
	enum TTASampleBytes samplebytes,
	unsigned int nchan
)
/*@modifies	*dest,
		*priv,
		user->is_new_frame,
		user->frame_is_finished,
		user->crc,
		user->ni32,
		user->ni32_total,
		user->nbytes_tta,
		user->nbytes_tta_total
@*/
;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_codecstate_priv_size                                             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              calculates the size of the private state struct             //
//                                                                          //
// return:                                                                  //
//              the size of the struct, or 0 on failure                     //
//                                                                          //
// parameters:                                                              //
//              nchan:                                                      //
//                      number of audio channels                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#undef nchan
size_t libttaR_codecstate_priv_size(unsigned int nchan)
/*@*/
;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_ttabuf_size                                                      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              calculates a buffer size for the dest/src buffer for        //
//      _tta_encode/_tta_decode, respectively                               //
//                                                                          //
// return:                                                                  //
//              size for a TTA buffer that "should" be safe                 //
//                                                                          //
// parameters:                                                              //
//              nsamples:                                                   //
//                      target number of PCM samples-per-channel to code    //
//              nchan:                                                      //
//                      number of audio channels                            //
//              samplesbytes:                                               //
//                      bytes-per-sample (1, 2, 3)                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#undef nsamples
#undef nchan
#undef samplebytes
extern size_t libttaR_ttabuf_size(
	size_t nsamples,
	unsigned int nchan,
	enum TTASampleBytes samplebytes
)
/*@*/
;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_test_nchan                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              tests whether libttaR was configured to support nchan audio //
//      channels                                                            //
//                                                                          //
// return:                                                                  //
//              true or false                                               //
//                                                                          //
// parameters:                                                              //
//              nchan:                                                      //
//                      number of audio channels                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#undef nchan
bool libttaR_test_nchan(unsigned int nchan)
/*@*/
;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_nsamples_perframe                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              calculates the number of samples-per-channel per TTA frame  //
//                                                                          //
// return:                                                                  //
//              the number of samples-per-channel per TTA frame             //
//                                                                          //
// parameters:                                                              //
//              samplerate:                                                 //
//                      audio sampling frequency in samples-per-second      //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#undef samplerate
extern size_t libttaR_nsamples_perframe(size_t samplerate)
/*@*/
;

#define libttaR_nsamples_perframe(samplerate) ( \
	(size_t) (TTA_FRAME_TIME * samplerate) \
)

//##########################################################################//


//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_pcm_read                                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              converts u8, i16le, or i24le to I32                         //
//                                                                          //
// return:                                                                  //
//              nsamples on success                                         //
//                                                                          //
// parameters:                                                              //
//              *dest:                                                      //
//                      I32 buffer                                          //
//              *src:                                                       //
//                      PCM buffer                                          //
//              nsamples:                                                   //
//                      number of PCM samples                               //
//              samplebytes:                                                //
//                      bytes-per-sample (1, 2, 3)                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#undef dest
#undef src
#undef nsamples
#undef samplebytes
extern size_t libttaR_pcm_read(
	int32_t *dest,
	const uint8_t *src,
	size_t nsamples,
	enum TTASampleBytes samplebytes
)
/*@modifies	*dest@*/
;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_pcm_write                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              converts I32 to u8, i16le, or i24le                         //
//                                                                          //
// return:                                                                  //
//              nsamples on success                                         //
//                                                                          //
// parameters:                                                              //
//              *dest:                                                      //
//                      PCM buffer                                          //
//              *src:                                                       //
//                      I32 buffer                                          //
//              nsamples:                                                   //
//                      number of I32 samples                               //
//              samplebytes:                                                //
//                      bytes-per-sample (1, 2, 3)                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#undef dest
#undef src
#undef nsamples
#undef samplebytes
extern size_t libttaR_pcm_write(
	uint8_t *dest,
	const int32_t *src,
	size_t nsamples,
	enum TTASampleBytes samplebytes
)
/*@modifies	*dest@*/
;

//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// libttaR_crc32                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// description:                                                             //
//              calculates a CRC with the following properties:             //
//                      width   : 32                                        //
//                      endian  : little                                    //
//                      poly    : 0xEDB88320u                               //
//                      xor-in  : 0xFFFFFFFFu                               //
//                      xor-out : 0xFFFFFFFFu                               //
//                                                                          //
// return:                                                                  //
//              the CRC                                                     //
//                                                                          //
// parameters:                                                              //
//              *buf:                                                       //
//                      the data to read                                    //
//              size:                                                       //
//                      size of the data                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
#undef buf
#undef size
extern uint32_t libttaR_crc32(const uint8_t *buf, size_t size)
/*@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
