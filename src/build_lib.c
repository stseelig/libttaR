/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// build_lib.c                                                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#ifndef S_SPLINT_S

#if __STDC_VERSION__ < 199901L
#error "compile with '-std=c99'"
#endif	/* __STDC_VERSION__ */

#endif	/* S_SPLINT_S */

/* //////////////////////////////////////////////////////////////////////// */

#define C_BUILD_C

#include "./lib/0-0_info.c"
#include "./lib/0-1_tables_rice24.c"
#include "./lib/0-2_tables_rice24_dec.c"
#include "./lib/0-3_tables_crc32.c"
#include "./lib/1-0_test_nchan.c"
#include "./lib/1-1_nsamples_perframe_tta1.c"
#include "./lib/1-2_ttabuf_safety_margin.c"
#include "./lib/1-3_codecstate_priv_size.c"
#include "./lib/1-4_crc32.c"
#include "./lib/2-0_pcm_read.c"
#include "./lib/2-1_pcm_write.c"
#include "./lib/3-0_tta_enc.c"
#include "./lib/3-1_tta_dec.c"

/* EOF //////////////////////////////////////////////////////////////////// */
