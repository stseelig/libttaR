#ifndef TTA_CODEC_SIMD_FILTER_PPC_H
#define TTA_CODEC_SIMD_FILTER_PPC_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/simd/filter.ppc.h                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      This is a straight port of the x86 version and has not been tuned   //
//                                                                          //
//      These functions "should" work no matter the endianness, but have    //
// only been tested on big-endian (PowerMacG4; POWER9 == $$$$$)             //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef S_SPLINT_S
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#error "powerpc little-endian is untested; comment to be a guinea pig"
#endif
#endif // S_SPLINT_S

//////////////////////////////////////////////////////////////////////////////

#ifndef S_SPLINT_S
#ifndef __BYTE_ORDER__
#error "'__BYTE_ORDER__' not defined"
#endif
#ifndef __ORDER_BIG_ENDIAN__
#error "'__ORDER_BIG_ENDIAN__' not defined"
#endif
#ifndef __ORDER_LITTLE_ENDIAN__
#error "'__ORDER_LITTLE_ENDIAN__' not defined"
#endif
#if (__BYTE_ORDER__ != __ORDER_BIG_ENDIAN__) \
 && (__BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__)
#error "__BYTE_ORDER__"
#endif

#ifndef __ALTIVEC__
#error "no AltiVec"
#endif
#endif // S_SPLINT_S

//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>	// true

#include <altivec.h>
#ifndef vector	// gcc
#define vector	__vector
#endif

#include "../../bits.h"

#include "../tta.h"

//////////////////////////////////////////////////////////////////////////////

// aligned in 'struct Codec'
struct Filter{
	i32	qm[8u];
	i32	dx[8u];
	i32	dl[8u];
	i32	error;	// the full error
};

//////////////////////////////////////////////////////////////////////////////

ALWAYS_INLINE CONST vector i32 predictz_vi32(vector i32, vector i32) /*@*/;
ALWAYS_INLINE CONST vector i32 cneg_izaz_vi32(vector i32, vector i32) /*@*/;

#ifndef NDEBUG
ALWAYS_INLINE CONST _Bool all_zero_vi32(vector i32) /*@*/;
#endif

ALWAYS_INLINE CONST i32 sum_vi32(vector i32) /*@*/;

ALWAYS_INLINE CONST vector i32 update_m_hi(vector i32) /*@*/;
ALWAYS_INLINE CONST vector i32 update_b_hi(vector i32, i32) /*@*/;
ALWAYS_INLINE CONST vector i32 update_mb_lo(vector i32, vector i32) /*@*/;

//////////////////////////////////////////////////////////////////////////////

#define FILTER_VARIABLES \
	i32 *const restrict filter_a = ASSUME_ALIGNED( filter->qm   , 16u); \
	i32 *const restrict error    = ASSUME_ALIGNED(&filter->error, 16u); \
	\
	i32 retval; \
	vector i32 a_lo, a_hi, m_lo, m_hi, b_lo, b_hi; \
	vector i32 r_lo, r_hi, t_lo, t_hi; \
	vector i32 m_lo_out, m_hi_out, b_lo_out, b_hi_out; \
	vector i32 v_error;

#define FILTER_READ { \
	a_lo     = vec_ld( 0, filter_a); \
	a_hi     = vec_ld(16, filter_a); \
	m_lo     = vec_ld(32, filter_a); \
	m_hi     = vec_ld(48, filter_a); \
	b_lo     = vec_ld(64, filter_a); \
	b_hi     = vec_ld(80, filter_a); \
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

#define FILTER_UPDATE_MB(Xvalue) { \
	m_hi_out = update_m_hi(b_hi); \
	b_hi_out = update_b_hi(b_hi, (Xvalue)); \
	m_lo_out = update_mb_lo(m_lo, m_hi); \
	b_lo_out = update_mb_lo(b_lo, b_hi); \
}

#define FILTER_WRITE { \
	vec_st(a_lo    ,  0, filter_a); \
	vec_st(a_hi    , 16, filter_a); \
	vec_st(m_lo_out, 32, filter_a); \
	vec_st(m_hi_out, 48, filter_a); \
	vec_st(b_lo_out, 64, filter_a); \
	vec_st(b_hi_out, 80, filter_a); \
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
	*error = retval;

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
	*error = value;

	return retval;
}

//--------------------------------------------------------------------------//

/**@fn predictz_vi32
 * @brief helps the CPU to know if it can skip the next few instructions
 *
 * @param x the input vector
 * @param error the extended error vector
 *
 * @return error != 0 ? x : 0
**/
ALWAYS_INLINE CONST vector i32
predictz_vi32(const vector i32 x, const vector i32 error)
/*@*/
{
	const vector i32 v_zero  = (const vector i32) { 0, 0, 0, 0 };
	const vector i32 v_isnez = (const vector i32) vec_cmpne(
		error, v_zero
	);

	return vec_and(v_isnez, x);
}

/**@fn cneg_izaz_vi32
 * @brief conditional negation. if zero then already zero
 *
 * @param x the input vector
 * @param cmp the comparison vector
 *
 * @return cmp < 0 ? -x : x
 *
 * @pre all_zero_vi32(cmp) ? all_zero_vi32(x) : true
**/
ALWAYS_INLINE CONST vector i32
cneg_izaz_vi32(const vector i32 x, const vector i32 cmp)
/*@*/
{
	const vector i32 v_zero  = (const vector i32) { 0, 0, 0, 0 };
	const vector i32 v_isltz = (const vector i32) vec_cmplt(cmp, v_zero);

	assert(all_zero_vi32(cmp) ? all_zero_vi32(x) : true);

	return vec_sub(vec_xor(x, v_isltz), v_isltz);
}

#ifndef NDEBUG
/**@fn all_zero_vi32
 * @brief checks that each item is not zero
 *
 * @param x the input vector
 *
 * @return true or false
 *
 * @note only used in an assertion
**/
ALWAYS_INLINE CONST _Bool
all_zero_vi32(vector i32 x)
/*@*/
{
	const vector i32 v_zero = (const vector i32) { 0, 0, 0, 0 };

	return (_Bool) vec_all_eq(x, v_zero);
}
#endif

/**@fn sum_vi32
 * @brief adds together every item in the vector
 *
 * @param x the input vector
 *
 * @return the sum of all items in the vector
**/
ALWAYS_INLINE CONST i32
sum_vi32(const vector i32 x)
/*@*/
{
	const vector i32 v_zero = (const vector i32) { 0, 0, 0, 0 };

	return (i32) vec_extract(vec_sums(x, v_zero), 3);
}

/**@fn update_m_hi
 * @brief updates the high half of 'm'
 *
 * @param b_hi the high half of 'b'
 *
 * @return the updated high half of 'm'
**/
ALWAYS_INLINE CONST vector i32
update_m_hi(vector i32 b_hi)
/*@*/
{
	const vector u32 v_sra = (const vector u32) {
		(u32) 30u, (u32) 30u, (u32) 30u, (u32) 30u
	};
	const vector i32 v_or  = (const vector i32) {
		(i32) 0x1, (i32) 0x1, (i32) 0x1, (i32) 0x1
	};
	const vector u32 v_sl  = (const vector u32) {
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
		(u32)  0u, (u32)  1u, (u32)  1u, (u32)  2u
#else // __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
		(u32)  2u, (u32)  1u, (u32)  1u, (u32)  0u
#endif
	};

	b_hi = vec_sra(b_hi, v_sra);
	b_hi = vec_or(b_hi, v_or);
	return vec_sl(b_hi, v_sl);
}

/**@fn update_b_hi
 * @brief updates the high half of 'b'
 *
 * @param b_hi the high half of 'b'
 * @param value input/output value from the filter
 *
 * @return the updated high half of 'b'
**/
ALWAYS_INLINE CONST vector i32
update_b_hi(const vector i32 b_hi, const i32 value)
/*@*/
{
	const vector i32 v_zero  = (const vector i32) { 0, 0, 0, 0 };
	const vector i32 v_in7   = vec_sld(b_hi, v_zero,  4u);
	const vector i32 v_in6   = vec_sld(b_hi, v_zero,  8u);
	const vector i32 v_in5   = vec_sld(b_hi, v_zero, 12u);
	const vector i32 v_value = vec_splats(value);

	return vec_sub(vec_sub(v_value, v_in7), vec_add(v_in5, v_in6));
}

/**@fn update_mb_lo
 * @brief updates the low half of 'm' or 'b'
 *   shift 'lo' left by one, then put the leftmost item in 'hi' into 'lo'
 *
 * @param mb_lo the low half
 * @param mb_hi the high half
 *
 * @return the updated low half
**/
ALWAYS_INLINE CONST vector i32
update_mb_lo(const vector i32 mb_lo, const vector i32 mb_hi)
/*@*/
{
	return vec_sld(mb_lo, mb_hi, 4u);
}

//////////////////////////////////////////////////////////////////////////////

// overloading keywords is screwed up (gcc Debian 12.2.0-13)
#undef  bool
#define bool	_Bool

// EOF ///////////////////////////////////////////////////////////////////////
#endif
