#ifndef TTA_CODEC_FILTER_FILTER__C_H
#define TTA_CODEC_FILTER_FILTER__C_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/filter/filter._C.h                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2007, Aleksander Djuric                                    //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../../bits.h"

//==========================================================================//

// aligned in 'struct Codec'
struct Filter {
	i32	qm[8u];
	i32	dx[9u];	// the extra value is for a memmove trick
	i32	dl[9u];	// ~
	i32	error;	// sign of the error (-1, 1, or 0)
};

//==========================================================================//

#include "../common.h"
#include "../tta.h"	// asr32

//////////////////////////////////////////////////////////////////////////////

#undef a
ALWAYS_INLINE i32 filter_sum_update_a(
	i32 *restrict a, const i32 *restrict, const i32 *restrict , i32, i32
)
/*@modifies	*a@*/
;

#undef m
ALWAYS_INLINE void filter_update_m(i32 *restrict m, const i32 *restrict)
/*@modifies	*m@*/
;

ALWAYS_INLINE CONST i32 updated_m(i32, bitcnt) /*@*/;

#undef b
ALWAYS_INLINE void filter_update_b(i32 *restrict b)
/*@modifies	*b@*/
;

#undef m
#undef b
ALWAYS_INLINE void filter_shift_mb(i32 *restrict m, i32 *restrict b)
/*@modifies	*m,
		*b
@*/
;

ALWAYS_INLINE CONST i32 signof32(i32) /*@*/;

//////////////////////////////////////////////////////////////////////////////

///@see "../filter.h"
ALWAYS_INLINE i32
tta_filter_enc(
	struct Filter *const restrict filter, const i32 value, i32 round,
	const bitcnt k
)
/*@modifies	*filter@*/
{
	i32 *const restrict a = ASSUME_ALIGNED( filter->qm   , 16u);
	i32 *const restrict m = ASSUME_ALIGNED( filter->dx   , 16u);
	i32 *const restrict b = ASSUME_ALIGNED( filter->dl   ,  4u);
	i32 *const restrict e = ASSUME_ALIGNED(&filter->error,  4u);

	i32 retval;

	round  = filter_sum_update_a(a, m, b, e[0u], round);
	filter_update_m(m, b);
	b[8u]  = value;
	filter_update_b(b);
	filter_shift_mb(m, b);
	retval = value - asr32(round, k);
	e[0u]  = signof32(retval);

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
	i32 *const restrict a = ASSUME_ALIGNED( filter->qm   , 16u);
	i32 *const restrict m = ASSUME_ALIGNED( filter->dx   , 16u);
	i32 *const restrict b = ASSUME_ALIGNED( filter->dl   ,  4u);
	i32 *const restrict e = ASSUME_ALIGNED(&filter->error,  4u);

	i32 retval;

	round  = filter_sum_update_a(a, m, b, e[0u], round);
	retval = value + asr32(round, k);
	b[8u]  = retval;
	filter_update_m(m, b);
	filter_update_b(b);
	filter_shift_mb(m, b);
	e[0u]  = signof32(value);

	return retval;
}

//--------------------------------------------------------------------------//

/**@fn filter_sum_update_a
 * @brief updates 'a' and sums the filter
 *
 * @param a[in out] filter->qm
 * @param m[in] filter->dx
 * @param b[in] filter->dl
 * @param error sign of the error (-1, 1, or 0)
 * @param round initial sum
 *
 * @return sum of the filter
**/
ALWAYS_INLINE i32
filter_sum_update_a(
	i32 *const restrict a, const i32 *const restrict m,
	const i32 *const restrict b, const i32 error, i32 round
)
/*@modifies	*a@*/
{
	uint i;
	for ( i = 0; i < 8u; ++i ){
		round += (a[i] += m[i] * error) * b[i];
	}
	return round;
}

/**@fn filter_update_m
 * @brief updates 'm'
 *
 * @param m[in out] filter->dx
 * @param b[in] filter->dl
**/
ALWAYS_INLINE void
filter_update_m(i32 *const restrict m, const i32 *const restrict b)
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
 * @param b filter->dl[]
 * @param k final shift amount
**/
ALWAYS_INLINE CONST i32
updated_m(const i32 b, const bitcnt k)
/*@*/
{
	return (i32) ((((u32) asr32(b, (bitcnt) 30u)) | 0x1u) << k);
}

/**@fn filter_update_b
 * @brief updates 'b'
 *
 * @param b[in out] filter->dl
**/
ALWAYS_INLINE void
filter_update_b(i32 *const restrict b)
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
 * @param m[in out] filter->dx
 * @param b[in out] filter->dl
**/
ALWAYS_INLINE void
filter_shift_mb(i32 *const restrict m, i32 *const restrict b)
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
 * @param x input value
 *
 * @retval -1, 1, or 0
 *
 * @note affected by LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
**/
ALWAYS_INLINE CONST i32
signof32(const i32 x)
/*@*/
{
#ifndef LIBTTAr_OPT_PREFER_CONDITIONAL_MOVES
	const u32 y = (u32) -x;

	if ( HAS_ASR(i32) ){
		return (i32) (((u32) asr32(x, (bitcnt) 31u)) | (y >> 31u));
	}
	else {	return (i32) (
			((u32) (-(i32) (((u32) x) >> 31u))) | (y >> 31u)
		);
	}
#else
	return (UNPREDICTABLE (x != 0)
		? (UNPREDICTABLE (x < 0) ? (i32) -1 : (i32) 1) : 0
	);
#endif
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
