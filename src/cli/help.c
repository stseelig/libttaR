//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// help.c                                                                   //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdio.h>

#include "cli.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
static const char help_usage[] = {
//1234567012345670123456701234567012345670123456701234567012345670123456701234
" [-options] INFILE... [-o OUTFILE]\n"
"\n"
};

/*@unchecked@*/
static const char help_tta2enc_opts[] = {
" Options:\n"
//1234567012345670123456701234567012345670123456701234567012345670123456701234
"\t"    "-?, --help\t\t\t"              "print this help\n"
"\t"    "-d, --delete-src\t\t"          "delete infiles\n"
"\t"    "-o, --outfile=FILE|DIR\t\t"    "set outfile name or directory\n"
"\t"    "-q, --quiet\t\t\t"             "only warnings and errors printed\n"
"\t"    "    --rawpcm=FMT,SR,NCHAN\t"   "rawpcm file stats\n"
"\t\t"          "FMT: u8, i16le, i24le\n"
};

/*@unchecked@*/
static const char help_tta2dec_opts[] = {
" Options:\n"
//1234567012345670123456701234567012345670123456701234567012345670123456701234
"\t"    "-?, --help\t\t\t"              "print this help\n"
"\t"    "-d, --delete-src\t\t"          "delete infiles\n"
"\t"    "-f, --format=FMT\t\t"          "outfile format\n"
"\t\t"          "FMT: raw, w64, wav\n"
"\t"    "-o, --outfile=FILE|DIR\t\t"    "set outfile name or directory\n"
"\t"    "-q, --quiet\t\t\t"             "only warnings and errors printed\n"
};

//////////////////////////////////////////////////////////////////////////////

void
errprint_help_tta2enc(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	errprint_program_intro();
	(void) fprintf(stderr, " Usage:\n");
	(void) fprintf(stderr, "\tttaR encode");
	(void) fputs(help_usage, stderr);
	(void) fputs(help_tta2enc_opts, stderr);
	return;
}

void
errprint_help_tta2dec(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	errprint_program_intro();
	(void) fprintf(stderr, " Usage:\n");
	(void) fprintf(stderr, "\tttaR decode");
	(void) fputs(help_usage, stderr);
	(void) fputs(help_tta2dec_opts, stderr);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
