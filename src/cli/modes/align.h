#ifndef H_TTA_MODES_ALIGN_H
#define H_TTA_MODES_ALIGN_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/align.h                                                            //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

/* workaround for C99 not having 'max_align_t' */
union x_max_alignment {
	long long ll;
	long double ld;
	void *p;
	void(*fp)(void);
};

#if __STDC_VERSION >= 201112L
#define ALIGNOF(x_x)	_Alignof
#elif defined(__GNUC__)
#define ALIGNOF(x_x)	__alignof__(x_x)
#else
#pragma message "compiler does not have an 'alignof'"
#define ALIGNOF(x_x)	(sizeof(union x_max_alignment))
#endif	/* ALIGNOF */

/* ======================================================================== */

/**@fn ALIGN_FW_DIFF
 * @brief align forward difference
 *
 * @param size  - input size
 * @param align - alignment
 *
 * @return number of bytes to add to a size
**/
#define ALIGN_FW_DIFF(x_size, x_alignment)	((size_t) ( \
	((x_size) % (x_alignment)) != 0 \
		? (x_alignment) - ((x_size) % (x_alignment)) : 0 \
))

/**@fn ALIGN_BW_DIFF
 * @brief align backward difference
 *
 * @param size  - input size
 * @param align - alignment
 *
 * @return number of bytes to subtract from a size
**/
#define ALIGN_BW_DIFF(x_size, x_alignment)	((size_t) ( \
	(x_size) % (x_alignment) \
))

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_MODES_ALIGN_H */
