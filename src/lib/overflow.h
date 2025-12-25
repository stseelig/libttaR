#ifndef H_TTA_CODEC_OVERFLOW_H
#define H_TTA_CODEC_OVERFLOW_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/overflow.h                                                         //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <limits.h>
#include <stddef.h>
#include <stdint.h>

#include "./common.h"

/* //////////////////////////////////////////////////////////////////////// */

#ifdef __GNUC__

#define X_BUILTIN_GNUC_UADD_OVERFLOW	__builtin_uadd_overflow
#define X_BUILTIN_GNUC_UADDL_OVERFLOW	__builtin_uaddl_overflow
#define X_BUILTIN_GNUC_UADDLL_OVERFLOW	__builtin_uaddll_overflow

#define X_BUILTIN_GNUC_UMUL_OVERFLOW	__builtin_umul_overflow
#define X_BUILTIN_GNUC_UMULL_OVERFLOW	__builtin_umull_overflow
#define X_BUILTIN_GNUC_UMULLL_OVERFLOW	__builtin_umulll_overflow

#else	/* !defined(__GNUC__) */

#define X_BUILTIN_GNUC_UADD_OVERFLOW	nil
#define X_BUILTIN_GNUC_UADDL_OVERFLOW	nil
#define X_BUILTIN_GNUC_UADDLL_OVERFLOW	nil

#define X_BUILTIN_GNUC_UMUL_OVERFLOW	nil
#define X_BUILTIN_GNUC_UMULL_OVERFLOW	nil
#define X_BUILTIN_GNUC_UMULLL_OVERFLOW	nil

#endif	/* __GNUC__ */

/* //////////////////////////////////////////////////////////////////////// */

/*@-mustdefine@*/	/* splint bug */

/* ======================================================================== */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_UADD_OVERFLOW)
#define X_ADD_UINT_OVERFLOW(x_res_ptr, x_a, x_b)	( \
	(int) X_BUILTIN_GNUC_UADD_OVERFLOW((x_a), (x_b), (x_res_ptr)) \
)
#else
ALWAYS_INLINE int
X_ADD_UINT_OVERFLOW(
	/*@out@*/ unsigned int *const res,
	const unsigned int a, const unsigned int b
)
/*@modifies	*res@*/
{
	*res = a + b;
	return (int) (*res < a);
}
#endif	/* X_ADD_UINT_OVERFLOW */
ALWAYS_INLINE int
add_uint_overflow(
	/*@out@*/ unsigned int *const res,
	const unsigned int a, const unsigned int b
)
/*@modifies	*res@*/
{
	return X_ADD_UINT_OVERFLOW(res, a, b);
}

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_UADDL_OVERFLOW)
#define X_ADD_ULONG_OVERFLOW(x_res_ptr, x_a, x_b)	( \
	(int) X_BUILTIN_GNUC_UADDL_OVERFLOW((x_a), (x_b), (x_res_ptr)) \
)
#else
ALWAYS_INLINE int
X_ADD_ULONG_OVERFLOW(
	/*@out@*/ unsigned long *const res,
	const unsigned long a, const unsigned long b
)
/*@modifies	*res@*/
{
	*res = a + b;
	return (int) (*res < a);
}
#endif	/* X_ADD_ULONG_OVERFLOW */
ALWAYS_INLINE int
add_ulong_overflow(
	/*@out@*/ unsigned long *const res,
	const unsigned long a, const unsigned long b
)
/*@modifies	*res@*/
{
	return X_ADD_ULONG_OVERFLOW(res, a, b);
}

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_UADDLL_OVERFLOW)
#define X_ADD_ULLONG_OVERFLOW(x_res_ptr, x_a, x_b)	( \
	(int) X_BUILTIN_GNUC_UADDLL_OVERFLOW((x_a), (x_b), (x_res_ptr)) \
)
#else
ALWAYS_INLINE int
X_ADD_ULLONG_OVERFLOW(
	/*@out@*/ unsigned long long *const res,
	const unsigned long long a, const unsigned long long b
)
/*@modifies	*res@*/
{
	*res = a + b;
	return (int) (*res < a);
}
#endif	/* X_ADD_ULLONG_OVERFLOW */
ALWAYS_INLINE int
add_ulonglong_overflow(
	/*@out@*/ unsigned long long *const res,
	const unsigned long long a, const unsigned long long b
)
/*@modifies	*res@*/
{
	return X_ADD_ULLONG_OVERFLOW(res, a, b);
}

/* ------------------------------------------------------------------------ */

#if SIZE_MAX == UINT_MAX
ALWAYS_INLINE int
add_usize_overflow(
	/*@out@*/ size_t *const res, const size_t a, const size_t b
)
/*@modifies	*res@*/
{
	return add_uint_overflow(
		(unsigned int *) res, (unsigned int) a, (unsigned int) b
	);
}
#elif SIZE_MAX == ULONG_MAX
ALWAYS_INLINE int
add_usize_overflow(
	/*@out@*/ size_t *const res, const size_t a, const size_t b
)
/*@modifies	*res@*/
{
	return add_ulong_overflow(
		(unsigned long *) res, (unsigned long) a, (unsigned long) b
	);
}
#elif SIZE_MAX == ULLONG_MAX
ALWAYS_INLINE int
add_usize_overflow(
	/*@out@*/ size_t *const res, const size_t a, const size_t b
)
/*@modifies	*res@*/
{
	return add_ulonglong_overflow(
		(unsigned long long *) res,
		(unsigned long long) a,
		(unsigned long long) b
	);
}
#else
#error "add_usize_overflow"
#endif	/* add_usize_overflow */

/* ======================================================================== */

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_UMUL_OVERFLOW)
#define X_MUL_UINT_OVERFLOW(x_res_ptr, x_a, x_b)	( \
	(int) X_BUILTIN_GNUC_UMUL_OVERFLOW((x_a), (x_b), (x_res_ptr)) \
)
#else
ALWAYS_INLINE int
X_MUL_UINT_OVERFLOW(
	/*@out@*/ unsigned int *const res,
	const unsigned int a, const unsigned int b
)
/*@modifies	*res@*/
{
	*res = a * b;
	return (int) (((a != 0) && (*res / a == b)) ? 0 : 1);
}
#endif	/* X_MUL_UINT_OVERFLOW */
ALWAYS_INLINE int
mul_uint_overflow(
	/*@out@*/ unsigned int *const res,
	const unsigned int a, const unsigned int b
)
/*@modifies	*res@*/
{
	return X_MUL_UINT_OVERFLOW(res, a, b);
}

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_UMULL_OVERFLOW)
#define X_MUL_ULONG_OVERFLOW(x_res_ptr, x_a, x_b)	( \
	(int) X_BUILTIN_GNUC_UMULL_OVERFLOW((x_a), (x_b), (x_res_ptr)) \
)
#else
ALWAYS_INLINE int
X_MUL_ULONG_OVERFLOW(
	/*@out@*/ unsigned long *const res,
	const unsigned long a, const unsigned long b
)
/*@modifies	*res@*/
{
	*res = a * b;
	return (int) (((a != 0) && (*res / a == b)) ? 0 : 1);
}
#endif	/* X_MUL_ULONG_OVERFLOW */
ALWAYS_INLINE int
mul_ulong_overflow(
	/*@out@*/ unsigned long *const res,
	const unsigned long a, const unsigned long b
)
/*@modifies	*res@*/
{
	return X_MUL_ULONG_OVERFLOW(res, a, b);
}

#if X_HAS_BUILTIN_GNUC(X_BUILTIN_GNUC_UMULLL_OVERFLOW)
#define X_MUL_ULLONG_OVERFLOW(x_res_ptr, x_a, x_b)	( \
	(int) X_BUILTIN_GNUC_UMULLL_OVERFLOW((x_a), (x_b), (x_res_ptr)) \
)
#else
ALWAYS_INLINE int
X_MUL_ULLONG_OVERFLOW(
	/*@out@*/ unsigned long long *const res,
	const unsigned long long a, const unsigned long long b
)
/*@modifies	*res@*/
{
	*res = a * b;
	return (int) (((a != 0) && (*res / a == b)) ? 0 : 1);
}
#endif	/* X_MUL_ULLONG_OVERFLOW */
ALWAYS_INLINE int
mul_ulonglong_overflow(
	/*@out@*/ unsigned long long *const res,
	const unsigned long long a, const unsigned long long b
)
/*@modifies	*res@*/
{
	return X_MUL_ULLONG_OVERFLOW(res, a, b);
}

/* ------------------------------------------------------------------------ */

#if SIZE_MAX == UINT_MAX
ALWAYS_INLINE int
mul_usize_overflow(
	/*@out@*/ size_t *const res, const size_t a, const size_t b
)
/*@modifies	*res@*/
{
	return mul_uint_overflow(
		(unsigned int *) res, (unsigned int) a, (unsigned int) b
	);
}
#elif SIZE_MAX == ULONG_MAX
ALWAYS_INLINE int
mul_usize_overflow(
	/*@out@*/ size_t *const res, const size_t a, const size_t b
)
/*@modifies	*res@*/
{
	return mul_ulong_overflow(
		(unsigned long *) res, (unsigned long) a, (unsigned long) b
	);
}
#elif SIZE_MAX == ULLONG_MAX
ALWAYS_INLINE int
mul_usize_overflow(
	/*@out@*/ size_t *const res, const size_t a, const size_t b
)
/*@modifies	*res@*/
{
	return mul_ulonglong_overflow(
		(unsigned long long *) res,
		(unsigned long long) a,
		(unsigned long long) b
	);
}
#else
#error "mul_usize_overflow"
#endif	/* mul_usize_overflow */

/* ======================================================================== */

/*@=mustdefine@*/

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_CODEC_OVERFLOW_H */
