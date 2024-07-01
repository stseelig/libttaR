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

#include <stddef.h>	// ptrdiff_t
#include <stdio.h>
#include <string.h>	// strchr

#include "../libttaR.h"

#include "cli.h"
#include "main.h"

//////////////////////////////////////////////////////////////////////////////

static void errprint_program_intro(
	const struct LibTTAr_VersionInfo *restrict,
	const struct LibTTAr_VersionInfo *restrict
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_ttaR_version(
	const char *restrict name,
	const struct LibTTAr_VersionInfo *restrict info
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_libstr_copyright(const char *restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

//////////////////////////////////////////////////////////////////////////////

/*@unchecked@*/
static const char intro_licence_blurb[] = {
//1234567012345670123456701234567012345670123456701234567012345670123456701234
"    This program is free software released under the terms of the GNU\n"
" General Public License v3. It is distributed in the hope that it will be\n"
" useful, but WITHOUT ANY WARRANTY. See the license for details.\n"
};

//--------------------------------------------------------------------------//

/*@unchecked@*/
static const char help_main[] = {
//1234567012345670123456701234567012345670123456701234567012345670123456701234
" Usage:\n"
"\t"    "ttaR MODE [-options] INFILE... [-o OUTFILE|OUTDIR]\n"
"\n"
"\t"    "ttaR MODE --help\n"
"\n"
" Modes:\n"
"\t"    "encode, decode\n"
};

/*@unchecked@*/
static const char help_mode_usage0_encode[] = {
//1234567012345670123456701234567012345670123456701234567012345670123456701234
" Usage:\n"
"\t"    "ttaR encode"
};

/*@unchecked@*/
static const char help_mode_usage0_decode[] = {
//1234567012345670123456701234567012345670123456701234567012345670123456701234
" Usage:\n"
"\t"    "ttaR decode"
};

/*@unchecked@*/
static const char help_mode_usage1[] = {
" [-options] INFILE... [-o OUTFILE]\n"
"\n"
};

//1234567012345670123456701234567012345670123456701234567012345670123456701234
#define OPT_COMMON_HELP \
"\t"    "-?, --help\t\t\t"              "print this help\n"

//1234567012345670123456701234567012345670123456701234567012345670123456701234
#define OPT_COMMON_SINGLE_THREADED \
"\t"    "-S, --single-threaded\t\t"     "can be more efficient than -M\n"
#define OPT_COMMON_MULTI_THREADED \
"    [*]\t" \
        "-M, --multi-threaded\t\t"      "with NPROCESSORS_ONLN threads\n"

//1234567012345670123456701234567012345670123456701234567012345670123456701234
#define OPT_COMMON_DELETE_SRC \
"\t"    "-d, --delete-src\t\t"          "delete each infile after coding\n"
#define OPT_COMMON_OUTFILE \
"\t"    "-o, --outfile=FILE|DIR\t\t"    "outfile name or directory\n"
#define OPT_COMMON_QUIET \
"\t"    "-q, --quiet\t\t\t"             "only warnings and errors printed\n"
#define OPT_COMMON_THREADS \
"\t"    "-t, --threads=N\t\t\t"         "multi-threaded with N threads\n"

//1234567012345670123456701234567012345670123456701234567012345670123456701234
#define OPT_ENCODE_RAWPCM \
"\t"    "    --rawpcm=FMT,SRATE,NCHAN\t""rawpcm file stats\n" \
"\t\t"          "FMT: u8, i16le, i24le\n"

//1234567012345670123456701234567012345670123456701234567012345670123456701234
#define OPT_DECODE_FORMAT \
"\t"    "-f, --format=FMT\t\t"          "outfile format\n" \
"\t\t"          "FMT: raw, [*] w64, wav\n"


/*@unchecked@*/
static const char help_mode_opts_encode[] = {
" Options:\n"
OPT_COMMON_HELP
"\n"
OPT_COMMON_SINGLE_THREADED
OPT_COMMON_MULTI_THREADED
"\n"
OPT_COMMON_DELETE_SRC
OPT_COMMON_OUTFILE
OPT_COMMON_QUIET
OPT_ENCODE_RAWPCM
OPT_COMMON_THREADS
};

/*@unchecked@*/
static const char help_mode_opts_decode[] = {
" Options:\n"
OPT_COMMON_HELP
"\n"
OPT_COMMON_SINGLE_THREADED
OPT_COMMON_MULTI_THREADED
"\n"
OPT_COMMON_DELETE_SRC
OPT_DECODE_FORMAT
OPT_COMMON_OUTFILE
OPT_COMMON_QUIET
OPT_COMMON_THREADS
};

//////////////////////////////////////////////////////////////////////////////

/**@fn errprint_help_main
 * @brief print main's help to stderr
**/
COLD void
errprint_help_main(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	errprint_program_intro(&ttaR_info, &libttaR_info);
	(void) fputs(help_main, stderr);
	return;
}

/**@fn errprint_help_mode_encode
 * @brief print the mode encode's help to stderr
**/
COLD void
errprint_help_mode_encode(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	errprint_program_intro(&ttaR_info, &libttaR_info);
	(void) fputs(help_mode_usage0_encode, stderr);
	(void) fputs(help_mode_usage1, stderr);
	(void) fputs(help_mode_opts_encode, stderr);
	return;
}

/**@fn errprint_help_mode_decode
 * @brief print the mode decode's help to stderr
**/
COLD void
errprint_help_mode_decode(void)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	errprint_program_intro(&ttaR_info, &libttaR_info);
	(void) fputs(help_mode_usage0_decode, stderr);
	(void) fputs(help_mode_usage1, stderr);
	(void) fputs(help_mode_opts_decode, stderr);
	return;
}

//--------------------------------------------------------------------------//

/**@fn errprint_program_intro
 * @brief print the version, copyright, and license info to stderr
 *
 * @param cli[in] the program's info struct
 * @param lib[in] the library's info struct
**/
static void
errprint_program_intro(
	const struct LibTTAr_VersionInfo *const restrict cli,
	const struct LibTTAr_VersionInfo *const restrict lib
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fputc('\n', stderr);
	errprint_ttaR_version("ttaR", cli);
	errprint_libstr_copyright(cli->copyright);
	errprint_ttaR_version("libttaR", lib);
	errprint_libstr_copyright(lib->copyright);
	(void) fputc('\n', stderr);
	(void) fputs(intro_licence_blurb, stderr);
	(void) fputc('\n', stderr);
	return;
}

/**@fn errprint_ttaR_version
 * @brief print the version info to stderr
 *
 * @param name[in] ttaR/libttaR
 * @param info[in] version info struct
**/
static void
errprint_ttaR_version(
	const char *const restrict name,
	const struct LibTTAr_VersionInfo *const restrict info
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fprintf(
		stderr, " %s %u.%u.%u", name, info->version,
		info->version_major, info->version_minor
	);
	if ( info->version_revis != 0 ){
		(void) fprintf(stderr, "-%u", info->version_revis);
	}
	if ( info->version_extra[0] != '\0' ){
		(void) fprintf(stderr, "~%s", info->version_extra);
	}
	(void) fprintf(stderr, " (%s)", info->version_date);
	(void) fputc('\n', stderr);
}

/**@fn errprint_libstr_copyrigth
 * @brief print the copyright string(s) to stderr
 *
 * @param str[in] the copyright string
**/
static void
errprint_libstr_copyright(const char *restrict str)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{

	const char *substr;
	ptrdiff_t diff;

	do {	substr = strchr(str, ';');
		if ( substr != NULL ){
			diff = (ptrdiff_t) (substr - str);
			(void) fprintf(stderr, "\t%.*s\n", (int) diff, str);
			str  = &str[diff + 1u];
		}
		else { (void) fprintf(stderr, "\t%s\n", str); }
	}
	while ( substr != NULL );

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
