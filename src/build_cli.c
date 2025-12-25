/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// build_cli.c                                                              //
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

#if !defined(_POSIX_C_SOURCE) || (_POSIX_C_SOURCE < 200809L)
#undef	_POSIX_C_SOURCE
#define _POSIX_C_SOURCE		200809L
#endif	/* _POSIX_C_SOURCE */

#if !defined(_FILE_OFFSET_BITS) || (_FILE_OFFSET_BITS < 64)
#undef	_FILE_OFFSET_BITS
#define _FILE_OFFSET_BITS	64
#endif	/* _FILE_OFFSET_BITS */

#endif	/* S_SPLINT_S */

/* //////////////////////////////////////////////////////////////////////// */

#define C_BUILD_C

#include "./cli/alloc.c"
#include "./cli/cli.c"
#include "./cli/debug.c"
#include "./cli/formats/guid.c"
#include "./cli/formats/metatags_skip.c"
#include "./cli/formats/tta1_check.c"
#include "./cli/formats/tta_seek.c"
#include "./cli/formats/tta_seek_check.c"
#include "./cli/formats/tta_write.c"
#include "./cli/formats/w64_check.c"
#include "./cli/formats/w64_write.c"
#include "./cli/formats/wav_check.c"
#include "./cli/formats/wav_write.c"
#include "./cli/help.c"
#include "./cli/main.c"
#include "./cli/modes/bufs.c"
#include "./cli/modes/mode_decode.c"
#include "./cli/modes/mode_decode_loop.c"
#include "./cli/modes/mode_encode.c"
#include "./cli/modes/mode_encode_loop.c"
#include "./cli/modes/mt-struct.c"
#include "./cli/open.c"
#include "./cli/opts/common.c"
#include "./cli/opts/decode.c"
#include "./cli/opts/encode.c"
#include "./cli/opts/optsget.c"

/* EOF //////////////////////////////////////////////////////////////////// */
