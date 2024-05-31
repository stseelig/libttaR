//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// cli.c                                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2024, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>

#include "../bits.h"
#include "../splint.h"

#include "debug.h"
#include "main.h"	// enum ProgramMode

//////////////////////////////////////////////////////////////////////////////

static void errprint_stats_infile(const char *const restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_stats_outfile(const char *const restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_stats_format(
	const struct FileStats *const restrict, enum ProgramMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_stats_frame(const struct FileStats *const restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_stats_pcm(double, size_t, size_t)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_stats_tta(double, size_t, size_t)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_stats_codectime(double, double, size_t)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

/*@observer@*/
static CONST const char *decfmt_name(enum DecFormat) /*@*/;

/*@observer@*/
static CONST const char *encfmt_name(enum EncFormat) /*@*/;

static CONST double calc_time_pcm(
	size_t, const struct FileStats *const restrict
)
/*@*/
;

static void errprint_size(size_t)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_time(double)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_chanmask_wav(uint, u32)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

//////////////////////////////////////////////////////////////////////////////

void
errprint_stats_precodec(
	const struct FileStats *const restrict fstat,
	const char *const restrict infile_name,
	const char *const restrict outfile_name, enum ProgramMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	errprint_stats_infile(infile_name);
	errprint_stats_outfile(outfile_name);
	errprint_stats_format(fstat, mode);
	errprint_stats_frame(fstat);
	return;
}

void
errprint_stats_postcodec(
	const struct FileStats *const restrict fstat,
	const struct EncStats *const restrict estat
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const size_t nbytes_pcm	= (size_t) (
		estat->nsamples_flat * fstat->samplebytes
	);
	const double pcmtime	= calc_time_pcm(estat->nsamples_flat, fstat);

	errprint_stats_pcm(pcmtime, estat->nframes, nbytes_pcm);
	errprint_stats_tta(pcmtime, nbytes_pcm, estat->nbytes_encoded);
	errprint_stats_codectime(pcmtime, estat->encodetime, nbytes_pcm);
	return;
}

void
errprint_runtime(double runtime, size_t files, enum ProgramMode mode)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char *mode_str;

	switch ( mode ){
	case MODE_ENCODE:
		mode_str = "encoded";
		break;
	case MODE_DECODE:
		mode_str = "decoded";
		break;
	}

	(void) fputc('\n', stderr);
	(void) fprintf(stderr, " %zu files %s (and written) in ",
		files, mode_str
	);
	errprint_time(runtime);
	// MAYBE print number of warnings
	(void) fputc('\n', stderr);
	return;
}

HOT void
errprint_spinner(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	const char spinner[] = {'|','\r','/','\r','-','\r','\\','\r'};
	static uchar i = 0;

	(void) fwrite(&spinner[i], (size_t) 2u, (size_t) 1u, stderr);
	i = (i + 2u != (uchar) (sizeof spinner) ? i + 2u : 0);
	return;
}

//==========================================================================//

static void
errprint_stats_infile(const char *const restrict name)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fprintf(stderr, " in\t: %s\n", name);
	return;
}

static void
errprint_stats_outfile(const char *const restrict name)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fprintf(stderr, " out\t: %s\n", name);
	return;
}

static void
errprint_stats_format(
	const struct FileStats *const restrict stats, enum ProgramMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char *inname, *outname;

	switch ( mode ){
	case MODE_ENCODE:
		inname	= decfmt_name(stats->decfmt);
		outname	= encfmt_name(stats->encfmt);
		break;
	case MODE_DECODE:
		inname	= encfmt_name(stats->encfmt);
		outname	= decfmt_name(stats->decfmt);
		break;
	}

	(void) fputs(" format\t: ", stderr);
	//
	(void) fputs(inname, stderr);
	(void) fputs(" => ", stderr);
	(void) fputs(outname, stderr);
	//
	(void) fputs("\t; ", stderr);
	(void) fprintf(stderr, "%c",
		stats->inttype == INT_SIGNED ? 'i' : 'u'
	);
	(void) fprintf(stderr, "%"PRIu16"", stats->samplebits);
	if ( stats->samplebits > (u16) 8u ){
		(void) fprintf(stderr, "%s",
			stats->endian == xENDIAN_LITTLE ? "le" : "be"
		);
	}
	//
	(void) fputs(" | ", stderr);
	(void) fprintf(stderr, "%"PRIu32" Hz", stats->samplerate);
	//
	(void) fputs(" | ", stderr);
	(void) fprintf(stderr, "%"PRIu16"-ch", stats->nchan);
	(void) fputs(" (", stderr);
	(void) errprint_chanmask_wav(
		(uint) stats->nchan, stats->chanmask_wav
	);
	(void) fputc(')', stderr);
	//
	(void) fputc('\n', stderr);
	return;
}

static void
errprint_stats_frame(const struct FileStats *const restrict stats)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fputs(" frame\t: ", stderr);
	(void) fprintf(stderr, "%zu sample%s",
		stats->framelen, stats->framelen == (size_t) 1u ? "" : "s"
	);
	//
	(void) fputs("\t; ", stderr);
	errprint_size((size_t) (stats->buflen * stats->samplebytes));
	(void) fputc('B', stderr);
	//
	(void) fputs("\t, ", stderr);
	errprint_time(calc_time_pcm(stats->buflen, stats));
	//
	(void) fputc('\n', stderr);
	return;
}

static void
errprint_stats_pcm(double pcmtime, size_t nframes, size_t nbytes_pcm)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fputs(" pcm\t: ", stderr);
	errprint_size(nbytes_pcm);
	(void) fputc('B', stderr);
	//
	(void) fputs("\t; ", stderr);
	(void) fprintf(stderr, "%zu frame%s",
		nframes, nframes == (size_t) 1u ? "" : "s"
	);
	//
	(void) fputs("\t, ", stderr);
	errprint_time(pcmtime);
	//
	(void) fputc('\n', stderr);
	return;
}

static void
errprint_stats_tta(double pcmtime, size_t nbytes_pcm, size_t nbytes_tta)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fputs(" tta\t: ", stderr);
	errprint_size(nbytes_tta);
	(void) fputc('B', stderr);
	//
	(void) fputs("\t; ", stderr);
	errprint_size((size_t) ((8u * nbytes_tta) / pcmtime));
	(void) fputs("b/s", stderr);
	//
	(void) fputs("\t, ", stderr);
	(void) fprintf(stderr, "%.3f", ((double) nbytes_tta) / nbytes_pcm);
	//
	(void) fputc('\n', stderr);
	return;
}

static void
errprint_stats_codectime(double pcmtime, double encodetime, size_t nbytes_pcm)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fputs(" time   : ", stderr);
	errprint_time(encodetime);
	//
	(void) fputs("\t; ", stderr);
	errprint_size((size_t) ((double) (8u * nbytes_pcm)) / encodetime);
	(void) fputs("b/s", stderr);
	//
	(void) fputs("\t, ", stderr);
	(void) fprintf(stderr, "%.1f", pcmtime / encodetime);
	(void) fputc('\n', stderr);
	//
	return;
}

//--------------------------------------------------------------------------//

/*@observer@*/
static CONST const char *
decfmt_name(enum DecFormat fmt)
/*@*/
{
	/*@observer@*/
	const char *const name[] = DECFMT_NAME_ARRAY;
	return name[fmt];
}

/*@observer@*/
static CONST const char *
encfmt_name(enum EncFormat fmt)
/*@*/
{
	/*@observer@*/
	const char *const name[] = xENCFMT_NAME_ARRAY;
	return name[fmt];
}

static CONST double
calc_time_pcm(size_t nsamples, const struct FileStats *const restrict stats)
/*@*/
{
	double size;
	size  = (double) nsamples;
	size /= (double) (stats->samplerate * stats->nchan);
	return size;
}

static void
errprint_size(size_t size)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	if ( size < (size_t) 1024u ){
		// bytes
		(void) fprintf(stderr, "%zu ", size);
	}
	else if ( size < (size_t) (1024u*1024u) ){
		// kibibytes
		(void) fprintf(stderr, "%.2f Ki", ((double) size) / 1024.0);
	}
	else if ( size < (size_t) (1024u*1024u*1024u) ){
		// mebibytes
		(void) fprintf(stderr, "%.2f Mi",
			((double) size) / (1024.0*1024.0)
		);
	}
	else {	// gibibytes
		(void) fprintf(stderr, "%.2f Gi",
			((double) size) / (1024.0*1024.0*1024.0)
		);
	}

	return;
}

static void
errprint_time(double sec)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const uint milli   =  ((uint) (sec  *  1000.0))  % 1000u;
	const uint seconds = (((uint)  sec) %  60u);
	const uint minutes = (((uint)  sec) /  60u)      % 60u;
	const uint hours   = (((uint)  sec) / (60u*60u)) % 24u;

	if ( hours > 0 ){
		(void) fprintf(stderr, "%uh%02um%02u.%03us",
			hours, minutes, seconds, milli
		);
	}
	else if ( minutes > 0 ){
		(void) fprintf(stderr, "%um%02u.%03us",
			minutes, seconds, milli
		);
	}
	else {	(void) fprintf(stderr, "%u.%03us", seconds, milli); }

	return;
}

static void
errprint_chanmask_wav(uint nchan, u32 mask)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	/*@observer@*/
	const char *const chan_name[] = WAVE_CHAN_NAMED_ARRAY;
	uint nchan_named;
	u32 curr_chan_bit;
	uint i;

	if ( (nchan == 1u) && ((mask == 0) || (mask == WAVE_CHAN_FC)) ){
		(void) fputs("mono", stderr);
		if ( mask == 0 ){
			(void) fputs(", assumed", stderr);
		}
	}
	else if ( (nchan == 2u)
	         &&
	          ((mask == 0) || (mask == (WAVE_CHAN_FL | WAVE_CHAN_FR)))
	){
		(void) fputs("stereo", stderr);
		if ( mask == 0 ){
			(void) fputs(", assumed", stderr);
		}
	}
	else {	nchan_named   = 0;
		curr_chan_bit = (u32) 0x1u;
		for ( i = 0; i < NUM_WAVE_CHAN_NAMED; ++i ){
			if ( (mask & curr_chan_bit) != 0 ){
				if ( nchan_named++ != 0 ){
					(void) fputc(',', stderr);
				}
				(void) fputs( chan_name[i], stderr);
			}
			curr_chan_bit <<= 1u;
		}
		if ( nchan_named != nchan ){
			if ( nchan_named == 0 ){
				(void) fputs("all", stderr);
			}
			else {	(void) fprintf(stderr, " + %u",
					nchan - nchan_named
				);
			}
			(void) fputs(" unassigned", stderr);
		}
	}

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
