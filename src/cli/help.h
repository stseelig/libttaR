#ifndef TTA_HELP_H
#define TTA_HELP_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// help.h                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

extern void errprint_help_tta2enc(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

extern void errprint_help_tta2dec(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
