#ifndef H_TTA_HELP_H
#define H_TTA_HELP_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// help.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "./common.h"

/* //////////////////////////////////////////////////////////////////////// */

COLD
BUILD_EXTERN NOINLINE void errprint_help_main(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

COLD
BUILD_EXTERN NOINLINE void errprint_help_mode_encode(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

COLD
BUILD_EXTERN NOINLINE void errprint_help_mode_decode(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_HELP_H */
