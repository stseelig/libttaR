#ifndef H_TTA_CODEC_FILTER_FILTER_ARM_H
#define H_TTA_CODEC_FILTER_FILTER_ARM_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter/filter.arm.h                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      This is a straight port of the x86/ppc version without tuning       //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <arm_neon.h>

#include "../common.h"
#include "../tta.h"
#include "../types.h"

#include "./asserts.h"
#include "./struct.h"

/* ======================================================================== */

#ifndef S_SPLINT_S

#ifndef __ARM_NEON
#error "__ARM_NEON"
#endif	/* __ARM_NEON */

#if __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#error "arm big-endian is untested; comment to be a guinea pig"
#endif	/* __BYTE_ORDER__ */

#endif /* S_SPLINT_S */

/* //////////////////////////////////////////////////////////////////////// */

CONST
ALWAYS_INLINE int32x4_t predictz_vi32(int32x4_t, int32x4_t) /*@*/;

CONST
ALWAYS_INLINE int32x4_t cneg_izaz_vi32(int32x4_t, int32x4_t) /*@*/;

CONST
ALWAYS_INLINE int32_t sum_vi32(int32x4_t) /*@*/;

CONST
ALWAYS_INLINE int32x4_t update_m_hi(int32x4_t) /*@*/;

CONST
ALWAYS_INLINE int32x4_t update_b_hi(int32x4_t, int32_t) /*@*/;

CONST
ALWAYS_INLINE int32x4_t update_mb_lo(int32x4_t, int32x4_t) /*@*/;

/* //////////////////////////////////////////////////////////////////////// */

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

#define FILTER_UPDATE_MB(x_value) { \
	m_hi_out = update_m_hi(b_hi); \
	b_hi_out = update_b_hi(b_hi, (x_value)); \
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

/* ------------------------------------------------------------------------ */

/**@fn predictz_vi32
 * @brief helps the CPU to know if it can skip the next few instructions
 *
 * @param x     - input vector
 * @param error - extended error vector
 *
 * @return error != 0 ? x : 0
**/
CONST
ALWAYS_INLINE int32x4_t
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
 * @param x   - input vector
 * @param cmp - comparison vector
 *
 * @return cmp < 0 ? -x : x
**/
CONST
ALWAYS_INLINE int32x4_t
cneg_izaz_vi32(const int32x4_t x, const int32x4_t cmp)
/*@*/
{
	const int32x4_t v_zero  = (const int32x4_t) { 0, 0, 0, 0 };
	const int32x4_t v_isltz = (const int32x4_t) vcltq_s32(cmp, v_zero);

	return vsubq_s32(veorq_s32(x, v_isltz), v_isltz);
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
sum_vi32(int32x4_t x)
/*@*/
{
	const int32x4_t v_zero = (const int32x4_t) { 0, 0, 0, 0 };

	x = vaddq_s32(x, vextq_s32(x, v_zero, 2));
	x = vaddq_s32(x, vextq_s32(x, v_zero, 1));

	return (int32_t) vgetq_lane_s32(x, 0);
}

/**@fn update_m_hi
 * @brief updates the high half of 'm'
 *
 * @param b_hi - high half of 'b'
 *
 * @return updated high half of 'm'
**/
CONST
ALWAYS_INLINE int32x4_t
update_m_hi(int32x4_t b_hi)
/*@*/
{
	const int32x4_t v_or  = (const int32x4_t) {
		INT32_C(0x1), INT32_C(0x1), INT32_C(0x1), INT32_C(0x1)
	};
	const int32x4_t v_sl  = (const int32x4_t) {
		INT32_C(  2), INT32_C(  1), INT32_C(  1), INT32_C(  0)
	};

	b_hi = vshrq_n_s32(b_hi, 30);
	b_hi = vorrq_s32(b_hi, v_or);

	return vshlq_s32(b_hi, v_sl);
}

/**@fn update_b_hi
 * @brief updates the high half of 'b'
 *
 * @param b_hi  - high half of 'b'
 * @param value - input/output value from the filter
 *
 * @return updated high half of 'b'
**/
CONST
ALWAYS_INLINE int32x4_t
update_b_hi(const int32x4_t b_hi, const int32_t value)
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
 * @param mb_lo - low half
 * @param mb_hi - high half
 *
 * @return updated low half
**/
CONST
ALWAYS_INLINE int32x4_t
update_mb_lo(const int32x4_t mb_hi, const int32x4_t mb_lo)
/*@*/
{
	return vextq_s32(mb_hi, mb_lo, 3);
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_FILTER_FILTER_ARM_H */
