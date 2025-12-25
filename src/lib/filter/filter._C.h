#ifndef H_TTA_CODEC_FILTER_FILTER__C_H
#define H_TTA_CODEC_FILTER_FILTER__C_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter/filter._C.h                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "../common.h"
#include "../tta.h"
#include "../types.h"

#include "./asserts.h"
#include "./struct.h"

/* //////////////////////////////////////////////////////////////////////// */

#undef a
ALWAYS_INLINE int32_t filter_sum_update_a(
	int32_t *RESTRICT a, const int32_t *RESTRICT, const int32_t *RESTRICT,
	int32_t, int32_t
)
/*@modifies	*a@*/
;

#undef m
ALWAYS_INLINE void filter_update_m(
	int32_t *RESTRICT m, const int32_t *RESTRICT
)
/*@modifies	*m@*/
;

CONST
ALWAYS_INLINE int32_t updated_m(int32_t, bitcnt) /*@*/;

#undef b
ALWAYS_INLINE void filter_update_b(int32_t *RESTRICT b)
/*@modifies	*b@*/
;

#undef m
#undef b
ALWAYS_INLINE void filter_shift_mb(int32_t *RESTRICT m, int32_t *RESTRICT b)
/*@modifies	*m,
		*b
@*/
;

CONST
ALWAYS_INLINE int32_t signof32(int32_t) /*@*/;

/* //////////////////////////////////////////////////////////////////////// */

/**@see "../filter.h" **/
ALWAYS_INLINE int32_t
tta_filter_enc(
	struct Filter *const RESTRICT filter, const int32_t value,
	int32_t round, const bitcnt k
)
/*@modifies	*filter@*/
{
	int32_t *const RESTRICT a = (
		ASSUME_ALIGNED(filter->qm, LIBTTAr_CODECSTATE_PRIV_ALIGN)
	);
	int32_t *const RESTRICT m = (
		ASSUME_ALIGNED(filter->dx, LIBTTAr_CODECSTATE_PRIV_ALIGN)
	);
	int32_t *const RESTRICT b = ASSUME_ALIGNED( filter->dl   , 4u);
	int32_t *const RESTRICT e = ASSUME_ALIGNED(&filter->error, 4u);
	/* * */
	int32_t retval;

	FILTER_ASSERTS_PRE;

	round  = filter_sum_update_a(a, m, b, e[0u], round);
	filter_update_m(m, b);
	b[8u]  = value;
	filter_update_b(b);
	filter_shift_mb(m, b);
	retval = value - asr32(round, k);
	e[0u]  = signof32(retval);

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
	int32_t *const RESTRICT a = (
		ASSUME_ALIGNED(filter->qm, LIBTTAr_CODECSTATE_PRIV_ALIGN)
	);
	int32_t *const RESTRICT m = (
		ASSUME_ALIGNED(filter->dx, LIBTTAr_CODECSTATE_PRIV_ALIGN)
	);
	int32_t *const RESTRICT b = ASSUME_ALIGNED( filter->dl   , 4u);
	int32_t *const RESTRICT e = ASSUME_ALIGNED(&filter->error, 4u);
	/* * */
	int32_t retval;

	FILTER_ASSERTS_PRE;

	round  = filter_sum_update_a(a, m, b, e[0u], round);
	retval = value + asr32(round, k);
	b[8u]  = retval;
	filter_update_m(m, b);
	filter_update_b(b);
	filter_shift_mb(m, b);
	e[0u]  = signof32(value);

	return retval;
}

/* ------------------------------------------------------------------------ */

/**@fn filter_sum_update_a
 * @brief updates 'a' and sums the filter
 *
 * @param a     - filter->qm
 * @param m     - filter->dx
 * @param b     - filter->dl
 * @param error - sign of the error (-1, 1, or 0)
 * @param round - initial sum
 *
 * @return sum of the filter
**/
ALWAYS_INLINE int32_t
filter_sum_update_a(
	int32_t *const RESTRICT a, const int32_t *const RESTRICT m,
	const int32_t *const RESTRICT b, const int32_t error, int32_t round
)
/*@modifies	*a@*/
{
	unsigned int i;

	for ( i = 0; i < 8u; ++i ){
		round += (a[i] += m[i] * error) * b[i];
	}
	return round;
}

/**@fn filter_update_m
 * @brief updates 'm'
 *
 * @param m - filter->dx
 * @param b - filter->dl
**/
ALWAYS_INLINE void
filter_update_m(int32_t *const RESTRICT m, const int32_t *const RESTRICT b)
/*@modifies	*m@*/
{
	m[8u] = updated_m(b[7u], (bitcnt) 2u);
	m[7u] = updated_m(b[6u], (bitcnt) 1u);
	m[6u] = updated_m(b[5u], (bitcnt) 1u);
	m[5u] = updated_m(b[4u], (bitcnt) 0u);

	return;
}

/**@fn updated_m
 * @brief returns an updated 'm' value
 *
 * @param b - filter->dl[]
 * @param k - final shift amount
**/
CONST
ALWAYS_INLINE int32_t
updated_m(const int32_t b, const bitcnt k)
/*@*/
{
	return (int32_t) ((((uint32_t) asr32(b, (bitcnt) 30u)) | 0x1u) << k);
}

/**@fn filter_update_b
 * @brief updates 'b'
 *
 * @param b - filter->dl
**/
ALWAYS_INLINE void
filter_update_b(int32_t *const RESTRICT b)
/*@modifies	*b@*/
{
	b[7u] = b[8u] - b[7u];
	b[6u] = b[7u] - b[6u];
	b[5u] = b[6u] - b[5u];

	return;
}

/**@fn filter_shift_mb
 * @brief shifts the 'm' and 'b' arrays left by 1
 *
 * @param m - filter->dx
 * @param b - filter->dl
**/
ALWAYS_INLINE void
filter_shift_mb(int32_t *const RESTRICT m, int32_t *const RESTRICT b)
/*@modifies	*m,
		*b
@*/
{
	MEMMOVE(m, &m[1u], (size_t) (8u * (sizeof *m)));
	MEMMOVE(b, &b[1u], (size_t) (8u * (sizeof *b)));

	return;
}

/**@fn signof32
 * @brief get the sign of a 32-bit integer
 *
 * @param x - input value
 *
 * @retval -1, 1, or 0
**/
CONST
ALWAYS_INLINE int32_t
signof32(const int32_t x)
/*@*/
{
#ifndef LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES

	const uint32_t y = -((uint32_t) x);

	if ( HAS_ASR(int32_t) ){
		return (int32_t) (
			((uint32_t) asr32(x, (bitcnt) 31u)) | (y >> 31u)
		);
	}
	else {	return (int32_t) ((-(((uint32_t) x) >> 31u)) | (y >> 31u)); }


#else	/* defined(LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES) */

	return (UNPREDICTABLE (x != 0)
		? (UNPREDICTABLE (x < 0) ? INT32_C(-1) : INT32_C(1)) : 0
	);

#endif	/* LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES */
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_FILTER_FILTER__C_H */
