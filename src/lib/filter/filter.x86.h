#ifndef TTA_CODEC_FILTER_FILTER_X86_H
#define TTA_CODEC_FILTER_FILTER_X86_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter/filter.x86.h                                                //
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
#endif // S_SPLINT_S

//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>	// true

#include <smmintrin.h>

#include "../../bits.h"

#include "../tta.h"	// asr32

//////////////////////////////////////////////////////////////////////////////

// aligned in 'struct Codec'
struct Filter{
	i32	qm[8u];
	i32	dx[8u];
	i32	dl[8u];
	i32	error;	// the full error
};

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE CONST __m128i predictz_m128i(__m128i, __m128i) /*@*/;
ALWAYS_INLINE CONST __m128i cneg_izaz_m128i_epi32(__m128i, __m128i) /*@*/;

#ifndef NDEBUG
ALWAYS_INLINE CONST i32 disjunct_m128i_epi32(__m128i) /*@*/;
#endif

ALWAYS_INLINE CONST __m128i mullo_m128i_epi32(__m128i, __m128i) /*@*/;
ALWAYS_INLINE CONST i32 sum_m128i_epi32(__m128i) /*@*/;

ALWAYS_INLINE CONST __m128i update_m_hi(__m128i) /*@*/;
ALWAYS_INLINE CONST __m128i update_b_hi(__m128i, i32) /*@*/;
ALWAYS_INLINE CONST __m128i update_mb_lo(__m128i, __m128i) /*@*/;

//////////////////////////////////////////////////////////////////////////////

#define FILTER_VARIABLES \
	i32 *const restrict filter_a = ASSUME_ALIGNED( filter->qm   , 16u); \
	i32 *const restrict error    = ASSUME_ALIGNED(&filter->error, 16u); \
	\
	i32 retval; \
	__m128i a_lo, a_hi, m_lo, m_hi, b_lo, b_hi; \
	__m128i r_lo, r_hi, t_lo, t_hi; \
	__m128i m_lo_out, m_hi_out, b_lo_out, b_hi_out; \
	__m128i v_error;

#define FILTER_READ { \
	a_lo     = _mm_load_si128((void *) &filter_a[ 0u]); \
	a_hi     = _mm_load_si128((void *) &filter_a[ 4u]); \
	m_lo     = _mm_load_si128((void *) &filter_a[ 8u]); \
	m_hi     = _mm_load_si128((void *) &filter_a[12u]); \
	b_lo     = _mm_load_si128((void *) &filter_a[16u]); \
	b_hi     = _mm_load_si128((void *) &filter_a[20u]); \
	v_error  = _mm_set1_epi32(*error); \
}

#define FILTER_SUM_UPDATE_A { \
	t_lo     = predictz_m128i(m_lo, v_error); \
	t_hi     = predictz_m128i(m_hi, v_error); \
	\
	t_lo     = cneg_izaz_m128i_epi32(t_lo, v_error); \
	a_lo     = _mm_add_epi32(a_lo, t_lo); \
	t_hi     = cneg_izaz_m128i_epi32(t_hi, v_error); \
	a_hi     = _mm_add_epi32(a_hi, t_hi); \
	\
	r_lo     = mullo_m128i_epi32(a_lo, b_lo); \
	r_hi     = mullo_m128i_epi32(a_hi, b_hi); \
	round   += sum_m128i_epi32(r_lo); \
	round   += sum_m128i_epi32(r_hi); \
}

#define FILTER_UPDATE_MB(Xvalue) { \
	m_hi_out = update_m_hi(b_hi); \
	b_hi_out = update_b_hi(b_hi, (Xvalue)); \
	m_lo_out = update_mb_lo(m_hi, m_lo); \
	b_lo_out = update_mb_lo(b_hi, b_lo); \
}

#define FILTER_WRITE { \
	_mm_store_si128((void *) &filter_a[ 0u], a_lo); \
	_mm_store_si128((void *) &filter_a[ 4u], a_hi); \
	_mm_store_si128((void *) &filter_a[ 8u], m_lo_out); \
	_mm_store_si128((void *) &filter_a[12u], m_hi_out); \
	_mm_store_si128((void *) &filter_a[16u], b_lo_out); \
	_mm_store_si128((void *) &filter_a[20u], b_hi_out); \
}

///@see "../filter.h"
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
	filter->error = retval;

	return retval;
}

///@see "../filter.h"
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
	filter->error = value;

	return retval;
}

//--------------------------------------------------------------------------//

/**@fn predictz_m128i
 * @brief helps the CPU to know if it can skip the next few instructions
 *
 * @param x the input vector
 * @param error the extended error vector
 *
 * @return error != 0 ? x : 0
 *
 * @note the error is 0 often enough that this provides a small speedup.
 *   it should be basically cost-free anyway
**/
ALWAYS_INLINE CONST __m128i
predictz_m128i(const __m128i x, const __m128i error)
/*@*/
{
	const __m128i v_iseqz = _mm_cmpeq_epi32(error, _mm_setzero_si128());

	return _mm_andnot_si128(v_iseqz, x);
}

/**@fn cneg_izaz_m128i_epi32
 * @brief conditional negation. if zero then already zero
 *
 * @param x the input vector
 * @param cmp the comparison vector
 *
 * @return cmp < 0 ? -x : x
 *
 * @pre disjunct_m128i_epi32(cmp) == 0 ? disjunct_m128i_epi32(x) == 0 : true
**/
ALWAYS_INLINE CONST __m128i
cneg_izaz_m128i_epi32(const __m128i x, const __m128i cmp)
/*@*/
{
#ifdef __SSSE3__

	assert(disjunct_m128i_epi32(cmp) == 0
		? disjunct_m128i_epi32(x) == 0 : true
	);

	return _mm_sign_epi32(x, cmp);	// SSSE3
#else
	const __m128i v_isltz = _mm_cmplt_epi32(cmp, _mm_setzero_si128());

	assert(disjunct_m128i_epi32(cmp) == 0
		? disjunct_m128i_epi32(x) == 0 : true
	);

	return _mm_sub_epi32(_mm_xor_si128(x, v_isltz), v_isltz);
#endif
}

#ifndef NDEBUG
/**@fn disjunct_m128i_epi32
 * @brief ors together every item in the vector
 *
 * @param x the input vector
 *
 * @return the disjunct of all items in the vector
 *
 * @note only used in an assertion
**/
ALWAYS_INLINE CONST i32
disjunct_m128i_epi32(__m128i x)
/*@*/
{
	x = _mm_or_si128(x, _mm_bsrli_si128(x, 8));
	x = _mm_or_si128(x, _mm_bsrli_si128(x, 4));
	return (i32) _mm_cvtsi128_si32(x);
}
#endif

/**@fn mullo_m128i_epi32
 * @brief multiplies the vectors and only keeps the lower 32 bits
 *
 * @param a the multiplicand
 * @param b the multiplier
 *
 * @return the lower 32-bit product
**/
ALWAYS_INLINE CONST __m128i
mullo_m128i_epi32(const __m128i a, const __m128i b)
/*@*/
{
#ifdef __SSE4_1__

	return _mm_mullo_epi32(a, b);	// SSE4.1
#else
	const __m128i p0 = _mm_mul_epu32(a, b);
	const __m128i p1 = _mm_mul_epu32(
		_mm_bsrli_si128(a, 4), _mm_bsrli_si128(b, 4)
	);

	return _mm_unpacklo_epi32(
		_mm_shuffle_epi32(p0, 0xE8), _mm_shuffle_epi32(p1, 0xE8)
	);
#endif
}

/**@fn sum_m128i_epi32
 * @brief adds together every item in the vector
 *
 * @param x the input vector
 *
 * @return the sum of all items in the vector
**/
ALWAYS_INLINE CONST i32
sum_m128i_epi32(__m128i x)
/*@*/
{
	x = _mm_add_epi32(x, _mm_bsrli_si128(x, 8));
	x = _mm_add_epi32(x, _mm_bsrli_si128(x, 4));
	return (i32) _mm_cvtsi128_si32(x);
}

/**@fn update_m_hi
 * @brief updates the high half of 'm'
 *
 * @param b_hi the high half of 'b'
 *
 * @return the updated high half of 'm'
**/
ALWAYS_INLINE CONST __m128i
update_m_hi(__m128i b_hi)
/*@*/
{
#ifdef __SSE4_1__

	__m128i t1, t2;

	b_hi = _mm_srai_epi32(b_hi, 30);
	b_hi = _mm_or_si128(b_hi, _mm_set1_epi32(1));

	// _mm_sllv_epi32() is super slow
	t1 = _mm_slli_epi32(b_hi, 1);
	t2 = _mm_slli_epi32(b_hi, 2);
	return _mm_castps_si128(_mm_blend_ps(	// SSE4.1
		_mm_blend_ps(			// SSE4.1
			_mm_castsi128_ps(b_hi), _mm_castsi128_ps(t1), 0x6
		),
		_mm_castsi128_ps(t2), 0x8
	));
#else
	__m128i t1, t2;

	b_hi = _mm_srai_epi32(b_hi, 30);
	b_hi = _mm_or_si128(b_hi, _mm_set1_epi32(1));

	t1   = _mm_slli_epi32(b_hi, 1);
	t2   = _mm_slli_epi32(b_hi, 2);

	b_hi = _mm_and_si128(
		_mm_set_epi32(0x00000000, 0x00000000, 0x00000000, 0xFFFFFFFF),
		b_hi
	);
	t1   = _mm_and_si128(
		_mm_set_epi32(0x00000000, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000),
		t1
	);
	t2   = _mm_and_si128(
		_mm_set_epi32(0xFFFFFFFF, 0x00000000, 0x00000000, 0x00000000),
		t2
	);
	return _mm_or_si128(_mm_or_si128(b_hi, t1), t2);
#endif
}

/**@fn update_b_hi
 * @brief updates the high half of 'b'
 *
 * @param b_hi the high half of 'b'
 * @param value input/output value from the filter
 *
 * @return the updated high half of 'b'
**/
ALWAYS_INLINE CONST __m128i
update_b_hi(const __m128i b_hi, const i32 value)
/*@*/
{
	const __m128i v_in7   = _mm_bsrli_si128(b_hi,  4);
	const __m128i v_in6   = _mm_bsrli_si128(b_hi,  8);
	const __m128i v_in5   = _mm_bsrli_si128(b_hi, 12);
	const __m128i v_value = _mm_set1_epi32(value);

	return _mm_sub_epi32(
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
ALWAYS_INLINE CONST __m128i
update_mb_lo(const __m128i mb_hi, const __m128i mb_lo)
/*@*/
{
#ifdef __SSSE3__

	return _mm_alignr_epi8(mb_hi, mb_lo, 4);	// SSSE3
#else
	return _mm_or_si128(
		_mm_bslli_si128(mb_hi, 12), _mm_bsrli_si128(mb_lo,  4)
	);
#endif
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
