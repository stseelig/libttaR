#ifndef TTA_CODEC_SIMD_FILTER_ARM_H
#define TTA_CODEC_SIMD_FILTER_ARM_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/simd/filter.arm.h                                                  //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      This is a straight port of the x86/ppc version without tuning       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifndef S_SPLINT_S
#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#error "arm big-endian is untested; comment to be a guinea pig"
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

#ifndef __ARM_NEON
#error "no NEON"
#endif
#endif // S_SPLINT_S

//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <stdbool.h>	// true

#include <arm_neon.h>

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

ALWAYS_INLINE CONST int32x4_t predictz_vi32(int32x4_t, int32x4_t) /*@*/;
ALWAYS_INLINE CONST int32x4_t cneg_izaz_vi32(int32x4_t, int32x4_t) /*@*/;

#ifndef NDEBUG
ALWAYS_INLINE CONST i32 disjunct_vi32(int32x4_t) /*@*/;
#endif

ALWAYS_INLINE CONST i32 sum_vi32(int32x4_t) /*@*/;

ALWAYS_INLINE CONST int32x4_t update_m_hi(int32x4_t) /*@*/;
ALWAYS_INLINE CONST int32x4_t update_b_hi(int32x4_t, i32) /*@*/;
ALWAYS_INLINE CONST int32x4_t update_mb_lo(int32x4_t, int32x4_t) /*@*/;

//////////////////////////////////////////////////////////////////////////////

#define FILTER_VARIABLES \
	i32 *const restrict filter_a = ASSUME_ALIGNED( filter->qm   , 16u); \
	i32 *const restrict error    = ASSUME_ALIGNED(&filter->error, 16u); \
	\
	i32 retval; \
	int32x4_t a_lo, a_hi, m_lo, m_hi, b_lo, b_hi; \
	int32x4_t r_lo, r_hi, t_lo, t_hi; \
	int32x4_t m_lo_out, m_hi_out, b_lo_out, b_hi_out; \
	int32x4_t v_error;

#define FILTER_READ { \
	a_lo     = vld1q_s32(&filter_a[ 0u]); \
	a_hi     = vld1q_s32(&filter_a[ 4u]); \
	m_lo     = vld1q_s32(&filter_a[ 8u]); \
	m_hi     = vld1q_s32(&filter_a[12u]); \
	b_lo     = vld1q_s32(&filter_a[16u]); \
	b_hi     = vld1q_s32(&filter_a[20u]); \
	v_error  = vdupq_n_s32(*error); \
}

#define FILTER_SUM_UPDATE_A { \
	t_lo     = predictz_vi32(m_lo, v_error); \
	t_hi     = predictz_vi32(m_hi, v_error); \
	\
	t_lo     = cneg_izaz_vi32(t_lo, v_error); \
	a_lo     = vaddq_s32(a_lo, t_lo); \
	t_hi     = cneg_izaz_vi32(t_hi, v_error); \
	a_hi     = vaddq_s32(a_hi, t_hi); \
	\
	r_lo     = vmulq_s32(a_lo, b_lo); \
	r_hi     = vmulq_s32(a_hi, b_hi); \
	round   += sum_vi32(r_lo); \
	round   += sum_vi32(r_hi); \
}

#define FILTER_UPDATE_MB(Xvalue) { \
	m_hi_out = update_m_hi(b_hi); \
	b_hi_out = update_b_hi(b_hi, (Xvalue)); \
	m_lo_out = update_mb_lo(m_hi, m_lo); \
	b_lo_out = update_mb_lo(b_hi, b_lo); \
}

#define FILTER_WRITE { \
	vst1q_s32(&filter_a[ 0u], a_lo); \
	vst1q_s32(&filter_a[ 4u], a_hi); \
	vst1q_s32(&filter_a[ 8u], m_lo_out); \
	vst1q_s32(&filter_a[12u], m_hi_out); \
	vst1q_s32(&filter_a[16u], b_lo_out); \
	vst1q_s32(&filter_a[20u], b_hi_out); \
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
ALWAYS_INLINE CONST int32x4_t
predictz_vi32(const int32x4_t x, const int32x4_t error)
/*@*/
{
	const int32x4_t v_zero  = (const int32x4_t) { 0, 0, 0, 0 };
	const int32x4_t v_iseqz = (const int32x4_t) vceqq_s32(error, v_zero);

	return vandq_s32(vmvnq_s32(v_iseqz), x);
}

/**@fn cneg_izaz_vi32
 * @brief conditional negation. if zero then already zero
 *
 * @param x the input vector
 * @param cmp the comparison vector
 *
 * @return cmp < 0 ? -x : x
 *
 * @pre disjunct_vi32(cmp) == 0 ? disjunct_vi32(x) == 0 : true
**/
ALWAYS_INLINE CONST int32x4_t
cneg_izaz_vi32(const int32x4_t x, const int32x4_t cmp)
/*@*/
{
	const int32x4_t v_zero  = (const int32x4_t) { 0, 0, 0, 0 };
	const int32x4_t v_isltz = (const int32x4_t) vcltq_s32(cmp, v_zero);

	assert(disjunct_vi32(cmp) == 0 ? disjunct_vi32(x) == 0 : true);

	return vsubq_s32(veorq_s32(x, v_isltz), v_isltz);
}

#ifndef NDEBUG
/**@fn disjunct_vi32
 * @brief ors together every item in the vector
 *
 * @param x the input vector
 *
 * @return the disjunct of all items in the vector
 *
 * @note only used in an assertion
**/
ALWAYS_INLINE CONST i32
disjunct_vi32(int32x4_t x)
/*@*/
{
	const int32x4_t v_zero = (const int32x4_t) { 0, 0, 0, 0 };

	x = vorrq_s32(x, vextq_s32(x, v_zero, 2));
	x = vorrq_s32(x, vextq_s32(x, v_zero, 1));
	return (i32) vgetq_lane_s32(x, 0);
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
sum_vi32(int32x4_t x)
/*@*/
{
	const int32x4_t v_zero = (const int32x4_t) { 0, 0, 0, 0 };

	x = vaddq_s32(x, vextq_s32(x, v_zero, 2));
	x = vaddq_s32(x, vextq_s32(x, v_zero, 1));
	return (i32) vgetq_lane_s32(x, 0);
}

/**@fn update_m_hi
 * @brief updates the high half of 'm'
 *
 * @param b_hi the high half of 'b'
 *
 * @return the updated high half of 'm'
**/
ALWAYS_INLINE CONST int32x4_t
update_m_hi(int32x4_t b_hi)
/*@*/
{
	const int32x4_t v_or  = (const int32x4_t) {
		(i32) 0x1, (i32) 0x1, (i32) 0x1, (i32) 0x1
	};
	const int32x4_t v_sl  = (const int32x4_t) {
		(i32)   2, (i32)   1, (i32)   1, (i32)   0
	};

	b_hi = vshrq_n_s32(b_hi, 30);
	b_hi = vorrq_s32(b_hi, v_or);
	return vshlq_s32(b_hi, v_sl);
}

/**@fn update_b_hi
 * @brief updates the high half of 'b'
 *
 * @param b_hi the high half of 'b'
 * @param value input/output value from the filter
 *
 * @return the updated high half of 'b'
**/
ALWAYS_INLINE CONST int32x4_t
update_b_hi(const int32x4_t b_hi, const i32 value)
/*@*/
{
	const int32x4_t v_zero  = (const int32x4_t) { 0, 0, 0, 0 };
	const int32x4_t v_in7   = vextq_s32(v_zero, b_hi, 3);
	const int32x4_t v_in6   = vextq_s32(v_zero, b_hi, 2);
	const int32x4_t v_in5   = vextq_s32(v_zero, b_hi, 1);
	const int32x4_t v_value = vdupq_n_s32(value);

	return vsubq_s32(vsubq_s32(v_value, v_in7), vaddq_s32(v_in5, v_in6));
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
ALWAYS_INLINE CONST int32x4_t
update_mb_lo(const int32x4_t mb_hi, const int32x4_t mb_lo)
/*@*/
{
	return vextq_s32(mb_hi, mb_lo, 3);
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
