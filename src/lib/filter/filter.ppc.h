#ifndef H_TTA_CODEC_FILTER_FILTER_PPC_H
#define H_TTA_CODEC_FILTER_FILTER_PPC_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter/filter.ppc.h                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      This is a straight port of the x86 version and has not been tuned   //
//                                                                          //
//      These functions "should" work no matter the endianness, but have    //
// only been tested on big-endian (PowerMacG4; POWER9 == $$$$$)             //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <altivec.h>
#ifndef vector	/* gcc */
#define vector	__vector
#endif	/* vector */

#include "../common.h"
#include "../tta.h"
#include "../types.h"

#include "./asserts.h"
#include "./struct.h"

/* ======================================================================== */

#ifndef S_SPLINT_S

#ifndef __ALTIVEC__
#error "__ALTIVEC__"
#endif	/* __ALTIVEC__ */

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#error "powerpc little-endian is untested; comment to be a guinea pig"
#endif	/* __BYTE_ORDER__ */

#endif	/* S_SPLINT_S */

//////////////////////////////////////////////////////////////////////////////

CONST
ALWAYS_INLINE vector int32_t predictz_vi32(vector int32_t, vector int32_t)
/*@*/
;

CONST
ALWAYS_INLINE vector int32_t cneg_izaz_vi32(vector int32_t, vector int32_t)
/*@*/
;

CONST
ALWAYS_INLINE int32_t sum_vi32(vector int32_t) /*@*/;

CONST
ALWAYS_INLINE vector int32_t update_m_hi(vector int32_t) /*@*/;

CONST
ALWAYS_INLINE vector int32_t update_b_hi(vector int32_t, int32_t) /*@*/;

CONST
ALWAYS_INLINE vector int32_t update_mb_lo(vector int32_t, vector int32_t)
/*@*/
;

//////////////////////////////////////////////////////////////////////////////

#define FILTER_VARIABLES \
	int32_t *const RESTRICT filter_a = ( \
		ASSUME_ALIGNED( \
			 filter->qm   , LIBTTAr_CODECSTATE_PRIV_ALIGN \
		) \
	); \
	int32_t *const RESTRICT error    = ( \
		ASSUME_ALIGNED( \
			&filter->error, LIBTTAr_CODECSTATE_PRIV_ALIGN \
		) \
	); \
	/* * */ \
	int32_t retval; \
	vector int32_t a_lo, a_hi, m_lo, m_hi, b_lo, b_hi; \
	vector int32_t r_lo, r_hi, t_lo, t_hi; \
	vector int32_t m_lo_out, m_hi_out, b_lo_out, b_hi_out; \
	vector int32_t v_error;

#define FILTER_READ { \
	a_lo     = vec_ld( 0LL, filter_a); \
	a_hi     = vec_ld(16LL, filter_a); \
	m_lo     = vec_ld(32LL, filter_a); \
	m_hi     = vec_ld(48LL, filter_a); \
	b_lo     = vec_ld(64LL, filter_a); \
	b_hi     = vec_ld(80LL, filter_a); \
	v_error  = vec_splats(*error); \
}

#define FILTER_SUM_UPDATE_A { \
	t_lo     = predictz_vi32(m_lo, v_error); \
	t_hi     = predictz_vi32(m_hi, v_error); \
	\
	t_lo     = cneg_izaz_vi32(t_lo, v_error); \
	a_lo     = vec_add(a_lo, t_lo); \
	t_hi     = cneg_izaz_vi32(t_hi, v_error); \
	a_hi     = vec_add(a_hi, t_hi); \
	\
	r_lo     = vec_mul(a_lo, b_lo); \
	r_hi     = vec_mul(a_hi, b_hi); \
	round   += sum_vi32(r_lo); \
	round   += sum_vi32(r_hi); \
}

#define FILTER_UPDATE_MB(x_value) { \
	m_hi_out = update_m_hi(b_hi); \
	b_hi_out = update_b_hi(b_hi, (x_value)); \
	m_lo_out = update_mb_lo(m_lo, m_hi); \
	b_lo_out = update_mb_lo(b_lo, b_hi); \
}

#define FILTER_WRITE { \
	vec_st(a_lo    ,  0LL, filter_a); \
	vec_st(a_hi    , 16LL, filter_a); \
	vec_st(m_lo_out, 32LL, filter_a); \
	vec_st(m_hi_out, 48LL, filter_a); \
	vec_st(b_lo_out, 64LL, filter_a); \
	vec_st(b_hi_out, 80LL, filter_a); \
}

/**@see "../filter.h" **/
ALWAYS_INLINE int32_t
tta_filter_enc(
	struct Filter *const RESTRICT filter, const int32_t value,
	int32_t round, const bitcnt k
)
/*@modifies	*filter@*/
{
	FILTER_VARIABLES;

	FILTER_ASSERTS_PRE;

	FILTER_READ;
	FILTER_SUM_UPDATE_A;
	FILTER_UPDATE_MB(value);
	FILTER_WRITE;
	retval = value - asr32(round, k);
	*error = retval;

	return retval;
}

/**@see "../filter.h" **/
ALWAYS_INLINE int32_t
tta_filter_dec(
	struct Filter *const RESTRICT filter, const int32_t value,
	int32_t round, const bitcnt k
)
/*@modifies	*filter@*/
{
	FILTER_VARIABLES;

	FILTER_ASSERTS_PRE;

	FILTER_READ;
	FILTER_SUM_UPDATE_A;
	retval = value + asr32(round, k);
	FILTER_UPDATE_MB(retval);
	FILTER_WRITE;
	*error = value;

	return retval;
}

//--------------------------------------------------------------------------//

/**@fn predictz_vi32
 * @brief helps the CPU to know if it can skip the next few instructions
 *
 * @param x     - input vector
 * @param error - extended error vector
 *
 * @return error != 0 ? x : 0
**/
CONST
ALWAYS_INLINE vector int32_t
predictz_vi32(const vector int32_t x, const vector int32_t error)
/*@*/
{
	const vector int32_t v_zero  = (const vector int32_t) { 0, 0, 0, 0 };
	const vector int32_t v_isnez = (const vector int32_t) (
		vec_cmpne(error, v_zero)
	);

	return vec_and(v_isnez, x);
}

/**@fn cneg_izaz_vi32
 * @brief conditional negation. if zero then already zero
 *
 * @param x   - input vector
 * @param cmp - comparison vector
 *
 * @return cmp < 0 ? -x : x
**/
CONST
ALWAYS_INLINE vector int32_t
cneg_izaz_vi32(const vector int32_t x, const vector int32_t cmp)
/*@*/
{
	const vector int32_t v_zero  = (const vector int32_t) { 0, 0, 0, 0 };
	const vector int32_t v_isltz = (const vector int32_t) (
		vec_cmplt(cmp, v_zero)
	);

	return vec_sub(vec_xor(x, v_isltz), v_isltz);
}

/**@fn sum_vi32
 * @brief adds together every item in the vector
 *
 * @param x - input vector
 *
 * @return sum of all items in the vector
**/
CONST
ALWAYS_INLINE int32_t
sum_vi32(const vector int32_t x)
/*@*/
{
	const vector int32_t v_zero = (const vector int32_t) { 0, 0, 0, 0 };

	return (int32_t) vec_extract(vec_sums(x, v_zero), 3);
}

/**@fn update_m_hi
 * @brief updates the high half of 'm'
 *
 * @param b_hi - high half of 'b'
 *
 * @return updated high half of 'm'
**/
CONST
ALWAYS_INLINE vector int32_t
update_m_hi(vector int32_t b_hi)
/*@*/
{
	const vector uint32_t v_sra = (const vector uint32_t) {
		UINT32_C( 30), UINT32_C( 30), UINT32_C( 30), UINT32_C( 30)
	};
	const vector  int32_t v_or  = (const vector  int32_t) {
		 INT32_C(0x1),  INT32_C(0x1),  INT32_C(0x1),  INT32_C(0x1)
	};
	const vector uint32_t v_sl  = (const vector uint32_t) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		UINT32_C(  0), UINT32_C(  1), UINT32_C(  1), UINT32_C(  2)
#else	/* __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__ */
		UINT32_C(  2), UINT32_C(  1), UINT32_C(  1), UINT32_C(  0)
#endif	/* __BYTE_ORDER__ */
	};

	b_hi = vec_sra(b_hi, v_sra);
	b_hi = vec_or(b_hi, v_or);

	return vec_sl(b_hi, v_sl);
}

/**@fn update_b_hi
 * @brief updates the high half of 'b'
 *
 * @param b_hi  - high half of 'b'
 * @param value - input/output value from the filter
 *
 * @return - updated high half of 'b'
**/
CONST
ALWAYS_INLINE vector int32_t
update_b_hi(const vector int32_t b_hi, const int32_t value)
/*@*/
{
	const vector int32_t v_zero  = (const vector int32_t) { 0, 0, 0, 0 };
	const vector int32_t v_in7   = vec_sld(b_hi, v_zero,  4u);
	const vector int32_t v_in6   = vec_sld(b_hi, v_zero,  8u);
	const vector int32_t v_in5   = vec_sld(b_hi, v_zero, 12u);
	const vector int32_t v_value = vec_splats(value);

	return vec_sub(vec_sub(v_value, v_in7), vec_add(v_in5, v_in6));
}

/**@fn update_mb_lo
 * @brief updates the low half of 'm' or 'b'
 *   shift 'lo' left by one, then put the leftmost item in 'hi' into 'lo'
 *
 * @param mb_lo - low half
 * @param mb_hi - high half
 *
 * @return updated low half
**/
CONST
ALWAYS_INLINE vector int32_t
update_mb_lo(const vector int32_t mb_lo, const vector int32_t mb_hi)
/*@*/
{
	return vec_sld(mb_lo, mb_hi, 4u);
}

/* //////////////////////////////////////////////////////////////////////// */

/* gcc-12 bug:
	overloading keywords is screwed up
*/
#undef  bool
#define bool	_Bool

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_FILTER_FILTER_PPC_H */
