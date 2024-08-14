#ifndef TTA_CODEC_FILTER_FILTER_X86_64_V3_H
#define TTA_CODEC_FILTER_FILTER_X86_64_V3_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter/filter.x86-64-v3.h                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      I have found no benefit in using 256-bit vectors                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef S_SPLINT_S
#ifndef __SSE2__
#error "no SSE2"
#endif
#ifndef __SSSE3__
#error "no SSSE3"
#endif
#ifndef __SSE4_1__
#error "no SSE4.1"
#endif
#ifndef __AVX2__
#error "no AVX2"
#endif
#endif // S_SPLINT_S

//////////////////////////////////////////////////////////////////////////////

#include <immintrin.h>

#include "../../bits.h"

//==========================================================================//

struct Filter{
	i32	qm[8u];
	i32	dx[8u];
	i32	dl[8u];
	i32	error;	// the full error
};

//==========================================================================//

#include "../common.h"
#include "../tta.h"

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE CONST i32 sum_m128i_epi32(__m128i) /*@*/;
ALWAYS_INLINE CONST __m128i update_m_hi(__m128i) /*@*/;
ALWAYS_INLINE CONST __m128i update_b_hi(__m128i, i32) /*@*/;
ALWAYS_INLINE CONST __m128i update_mb_lo(__m128i, __m128i) /*@*/;

//////////////////////////////////////////////////////////////////////////////

#define FILTER_VARIABLES \
	i32 *const restrict filter_a = filter->qm; \
	i32 *const restrict filter_m = filter->dx; \
	i32 *const restrict filter_b = filter->dl; \
	i32 *const restrict error    = &filter->error; /* the full error */ \
\
	i32 retval; \
	__m128i a_lo, a_hi, m_lo, m_hi, b_lo, b_hi; \
	__m128i r_lo, r_hi, t_lo, t_hi; \
	__m128i m_lo_out, m_hi_out, b_lo_out, b_hi_out; \
	__m128i v_error, v_mask; \
	const __m128i v_zero  = _mm_set1_epi32(0);

#define FILTER_READ { \
	a_lo    = _mm_loadu_si128((void *) &filter_a[0u]); \
	a_hi    = _mm_loadu_si128((void *) &filter_a[4u]); \
	m_lo    = _mm_loadu_si128((void *) &filter_m[0u]); \
	m_hi    = _mm_loadu_si128((void *) &filter_m[4u]); \
	b_lo    = _mm_loadu_si128((void *) &filter_b[0u]); \
	b_hi    = _mm_loadu_si128((void *) &filter_b[4u]); \
	v_error = _mm_set1_epi32(*error); \
	v_mask  = _mm_cmpeq_epi32(v_error, v_zero); \
}

#define FILTER_SUM_UPDATE_A { \
	t_lo   = _mm_sign_epi32(m_lo, v_error);	/* SSSE3 */ \
	t_hi   = _mm_sign_epi32(m_hi, v_error);	/* SSSE3 */ \
	t_lo   = _mm_andnot_si128(v_mask, t_lo); \
	t_hi   = _mm_andnot_si128(v_mask, t_hi); \
	a_lo   = _mm_add_epi32(a_lo, t_lo); \
	a_hi   = _mm_add_epi32(a_hi, t_hi); \
	r_lo   = _mm_mullo_epi32(a_lo, b_lo); 	/* SSE4.1 */ \
	r_hi   = _mm_mullo_epi32(a_hi, b_hi);	/* SSE4.1 */ \
	round += sum_m128i_epi32(r_lo); \
	round += sum_m128i_epi32(r_hi); \
}

#define FILTER_UPDATE_MB(value) { \
	m_hi_out = update_m_hi(b_hi); \
	b_hi_out = update_b_hi(b_hi, (value)); \
	m_lo_out = update_mb_lo(m_hi, m_lo); \
	b_lo_out = update_mb_lo(b_hi, b_lo); \
}

#define FILTER_WRITE { \
	_mm_storeu_si128((void *) &filter_a[0u], a_lo); \
	_mm_storeu_si128((void *) &filter_a[4u], a_hi); \
	_mm_storeu_si128((void *) &filter_m[0u], m_lo_out); \
	_mm_storeu_si128((void *) &filter_m[4u], m_hi_out); \
	_mm_storeu_si128((void *) &filter_b[0u], b_lo_out); \
	_mm_storeu_si128((void *) &filter_b[4u], b_hi_out); \
}

///@see "../filter.h"
// SSE2, SSSE3, SSE4.1
ALWAYS_INLINE i32
tta_filter_enc(
	struct Filter *const restrict filter, const i32 value, i32 round,
	const bitcnt k
)
/*@modifies	*filter@*/
{
	FILTER_VARIABLES;

	FILTER_READ;
	FILTER_SUM_UPDATE_A;
	FILTER_UPDATE_MB(value);
	FILTER_WRITE;
	retval = value - asr32(round, k);
	*error = retval;
	return retval;
}

///@see "../filter.h"
// SSE2, SSSE3, SSE4.1
ALWAYS_INLINE i32
tta_filter_dec(
	struct Filter *const restrict filter, const i32 value, i32 round,
	const bitcnt k
)
/*@modifies	*filter@*/
{
	FILTER_VARIABLES;

	FILTER_READ;
	FILTER_SUM_UPDATE_A;
	retval = value + asr32(round, k);
	FILTER_UPDATE_MB(retval);
	FILTER_WRITE;
	*error = value;
	return retval;
}

//--------------------------------------------------------------------------//

/**@fn sum_m128i_epi32
 * @brief sums every item in the vector
 *
 * @param x the input vector
 *
 * @return the sum of all items in the vector
**/
// SSE2
ALWAYS_INLINE CONST i32
sum_m128i_epi32(__m128i x)
/*@*/
{
	__m128i t;

	t = _mm_bsrli_si128(x, 8);
	x = _mm_add_epi32(t, x);
	t = _mm_bsrli_si128(x, 4);
	x = _mm_add_epi32(x, t);
	return (i32) _mm_cvtsi128_si32(x);
}

/**@fn update_m_hi
 * @brief updates the high half of 'm'
 *
 * @param b_hi the high half of 'b'
 *
 * @return the updated high half of 'm'
**/
// SSE2, AVX2
ALWAYS_INLINE CONST __m128i
update_m_hi(__m128i b_hi)
/*@*/
{
	const __m128i v_ormask = _mm_set1_epi32(1);
	const __m128i v_sllcnt = _mm_set_epi32(2, 1, 1, 0);

	b_hi = _mm_srai_epi32(b_hi, 30);
	b_hi = _mm_or_si128(b_hi, v_ormask);
	b_hi = _mm_sllv_epi32(b_hi, v_sllcnt);	// AVX2
	return b_hi;
}

/**@fn update_b_hi
 * @brief updates the high half of 'b'
 *
 * @param b_hi the high half of 'b'
 * @param value input/output value from the filter
 *
 * @return the updated high half of 'b'
**/
// SSE2
ALWAYS_INLINE CONST __m128i
update_b_hi(const __m128i b_hi, const i32 value)
/*@*/
{
	const __m128i v_in7   = _mm_bsrli_si128(b_hi,  4);
	const __m128i v_in6   = _mm_bsrli_si128(b_hi,  8);
	const __m128i v_in5   = _mm_bsrli_si128(b_hi, 12);
	const __m128i v_value = _mm_set1_epi32(value);

	return  _mm_sub_epi32(
		_mm_sub_epi32(v_value, v_in7), _mm_add_epi32(v_in5, v_in6)
	);
}

/**@fn update_mb_lo
 * @brief updates the low half of 'm' or 'b'
 *   shift 'lo' right by one, then put the lowest item in 'hi' into 'lo'
 *
 * @param mb_hi the high half
 * @param mb_lo the low half
 *
 * @return the updated low half
**/
// SSSE3
ALWAYS_INLINE CONST __m128i
update_mb_lo(const __m128i mb_hi, const __m128i mb_lo)
/*@*/
{
	return _mm_alignr_epi8(mb_hi, mb_lo, 4);	// SSSE3
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
