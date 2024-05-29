#ifndef TTA_HELP_H
#define TTA_HELP_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// help.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

extern COLD void errprint_help_main(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

extern COLD void errprint_help_mode_encode(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

extern COLD void errprint_help_mode_decode(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
