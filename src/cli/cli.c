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
static const char *decfmt_name(enum DecFormat)
/*@*/
;

/*@observer@*/
static const char *encfmt_name(enum EncFormat)
/*@*/
;

static double calc_time_pcm(size_t, const struct FileStats *const restrict)
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

void
errprint_spinner(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
{
	const char spinner[] = {'|','\r','/','\r','-','\r','\\','\r'};
	static u8 i = 0;

	(void) fwrite(&spinner[i], (size_t) 2u, (size_t) 1u, stderr);
	i = ((u8) (i + 2u) != (u8) (sizeof spinner) ? (u8) (i + 2u) : 0);
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

	(void) fprintf(stderr, " format\t: ");
	//
	(void) fputs(inname, stderr);
	(void) fprintf(stderr, " => ");
	(void) fputs(outname, stderr);
	//
	(void) fprintf(stderr, "\t; ");
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
	(void) fprintf(stderr, " | ");
	(void) fprintf(stderr, "%"PRIu32" Hz", stats->samplerate);
	//
	(void) fprintf(stderr, " | ");
	(void) fprintf(stderr, "%"PRIu16"-ch", stats->nchan);
	(void) fprintf(stderr, " (");
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
	(void) fprintf(stderr, " frame\t: ");
	(void) fprintf(stderr, "%zu sample%s",
		stats->framelen, stats->framelen == (size_t) 1u ? "" : "s"
	);
	//
	(void) fprintf(stderr, "\t; ");
	errprint_size((size_t) (stats->buflen * stats->samplebytes));
	(void) fputc('B', stderr);
	//
	(void) fprintf(stderr, "\t, ");
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
	(void) fprintf(stderr, " pcm\t: ");
	errprint_size(nbytes_pcm);
	(void) fputc('B', stderr);
	//
	(void) fprintf(stderr, "\t; ");
	(void) fprintf(stderr, "%zu frame%s",
		nframes, nframes == (size_t) 1u ? "" : "s"
	);
	//
	(void) fprintf(stderr, "\t, ");
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
	(void) fprintf(stderr, " tta\t: ");
	errprint_size(nbytes_tta);
	(void) fputc('B', stderr);
	//
	(void) fprintf(stderr, "\t; ");
	errprint_size((size_t) ((8u * nbytes_tta) / pcmtime));
	(void) fprintf(stderr, "b/s");
	//
	(void) fprintf(stderr, "\t, ");
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
	(void) fprintf(stderr, " time   : ");
	errprint_time(encodetime);
	//
	(void) fprintf(stderr, "\t; ");
	errprint_size((size_t) ((double) (8u * nbytes_pcm)) / encodetime);
	(void) fprintf(stderr, "b/s");
	//
	(void) fprintf(stderr, "\t, ");
	(void) fprintf(stderr, "%.1f", pcmtime / encodetime);
	(void) fputc('\n', stderr);
	//
	return;
}

//--------------------------------------------------------------------------//

/*@observer@*/
static const char *
decfmt_name(enum DecFormat fmt)
/*@*/
{
	const char *r;

	switch( fmt ){
	case DECFMT_RAWPCM:
		r = "raw";
		break;
	case DECFMT_W64:
		r = "w64";
		break;
	case DECFMT_WAV:
		r = "wav";
		break;
	}
	return r;
}

/*@observer@*/
static const char *
encfmt_name(enum EncFormat fmt)
/*@*/
{
	const char *r;

	switch( fmt ){
	case ENCFMT_TTA1:
		r = "tta1";
		break;
	}
	return r;
}

static double
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
	uint i = 0;

	if ( nchan == 1u ){
		switch ( mask ){
		default:
			break;
		case 0:
		case WAVE_CHAN_FC:
			(void) fprintf(stderr, "mono");
			if ( mask == 0 ){
				(void) fprintf(stderr, ", assumed");
			}
			return;
		}
	}
	if ( nchan == 2u ){
		switch ( mask ){
		default:
			break;
		case 0:
		case (WAVE_CHAN_FL | WAVE_CHAN_FR):
			(void) fprintf(stderr, "stereo");
			if ( mask == 0 ){
				(void) fprintf(stderr, ", assumed");
			}
			return;
		}
	}

	if ( (mask & WAVE_CHAN_FL)  != 0 ){
		(void) fprintf(stderr, "FL");
		++i;
	}
	if ( (mask & WAVE_CHAN_FR)  != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "FR");
	}
	if ( (mask & WAVE_CHAN_FC)  != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "FC");
	}
	if ( (mask & WAVE_CHAN_LFE) != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "LFE");
	}
	if ( (mask & WAVE_CHAN_BL)  != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "BL");
	}
	if ( (mask & WAVE_CHAN_BR)  != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "BR");
	}
	if ( (mask & WAVE_CHAN_FLC) != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "FLC");
	}
	if ( (mask & WAVE_CHAN_FRC) != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "FRC");
	}
	if ( (mask & WAVE_CHAN_BC)  != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "BC");
	}
	if ( (mask & WAVE_CHAN_SL)  != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "SL");
	}
	if ( (mask & WAVE_CHAN_SR)  != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "SR");
	}
	if ( (mask & WAVE_CHAN_TC)  != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "TC");
	}
	if ( (mask & WAVE_CHAN_TFL) != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "TFL");
	}
	if ( (mask & WAVE_CHAN_TFC) != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "TFC");
	}
	if ( (mask & WAVE_CHAN_TFR) != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "TFR");
	}
	if ( (mask & WAVE_CHAN_TBL) != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "TBL");
	}
	if ( (mask & WAVE_CHAN_TBC) != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "TBC");
	}
	if ( (mask & WAVE_CHAN_TBR) != 0 ){
		if ( i++ != 0 ){
			(void) fputc(',', stderr);
		}
		(void) fprintf(stderr, "TBR");
	}

	if ( i != nchan ){
		if ( i == 0 ){
			(void) fprintf(stderr, "all");
		}
		else {	(void) fprintf(stderr, " + %u", nchan - i); }
		(void) fprintf(stderr, " unassigned");
	}

	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
