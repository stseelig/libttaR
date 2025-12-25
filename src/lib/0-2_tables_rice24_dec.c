/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// codec/tables_rice24_dec.c                                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "./common.h"
#include "./types.h"

/* //////////////////////////////////////////////////////////////////////// */

#ifdef USE_TBCNT8_TABLE
/**@var tbcnt8_table
 * @brief trailing bit count 8-bit lookup table
**/
/*@unchecked@*/
BUILD_HIDDEN
const bitcnt_dec tbcnt8_table[256u] = {
#define T	bitcnt_dec
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 4u,	/* 0F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 5u,	/* 1F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 4u,	/* 2F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 6u,	/* 3F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 4u,	/* 4F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 5u,	/* 5F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 4u,	/* 6F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 7u,	/* 7F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 4u,	/* 8F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 5u,	/* 9F */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 4u,	/* AF */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 6u,	/* BF */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 4u,	/* CF */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 5u,	/* DF */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 4u,	/* EF */
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 3u,
(T) 0u, (T) 1u, (T) 0u, (T) 2u, (T) 0u, (T) 1u, (T) 0u, (T) 8u	/* FF */
#undef T
};
#endif /* USE_TBCNT8_TABLE */

/* EOF //////////////////////////////////////////////////////////////////// */
