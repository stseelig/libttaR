//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// cli.c                                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#ifdef S_SPLINT_S
#include "../splint.h"
#endif

/* ------------------------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "../bits.h"

#include "debug.h"
#include "main.h"	// enum ProgramMode

//////////////////////////////////////////////////////////////////////////////

static void errprint_stats_infile(const char *restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_stats_outfile(const char *restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

static void errprint_stats_format(
	const struct FileStats *restrict, enum ProgramMode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

#if 0
static void errprint_stats_frame(const struct FileStats *restrict)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;
#endif

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
	size_t, const struct FileStats *restrict
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

#if 0
static void errprint_chanmask_wav(uint, u32)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;
#endif

//////////////////////////////////////////////////////////////////////////////

/**@fn errprint_stats_precodec
 * @brief prints file stats to stderr before coding
 *
 * @param fstat[in] the bloated file stats struct
 * @param infile_name[in] the name of the source file
 * @param outfile_name[in] the name of the destination file
 * @param mode encode or decode
**/
void
errprint_stats_precodec(
	const struct FileStats *const restrict fstat,
	const char *const restrict infile_name,
	const char *const restrict outfile_name, const enum ProgramMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	errprint_stats_infile(infile_name);
	errprint_stats_outfile(outfile_name);
	errprint_stats_format(fstat, mode);
	//errprint_stats_frame(fstat);
	return;
}

/**@fn errprint_stat_postcodec
 * @brief print codec stats to stderr after coding
 *
 * @param fstat[in] the bloated file stats struct
 * @param estat[in] the encode stats struct
**/
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

/**@fn errprint_runtime
 * @brief print the total program runtime to stderr when coding multiple files
 *
 * @param runtime the total runtime
 * @param nfiles the total number of files coded
 * @param mode encode or decode
**/
void
errprint_runtime(
	const double runtime, const size_t nfiles, const enum ProgramMode mode
)
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
		nfiles, mode_str
	);
	errprint_time(runtime);
	// MAYBE print number of warnings
	(void) fputc('\n', stderr);
	return;
}

/**@fn errprint_spinner
 * @brief print the progress spinner to stderr
**/
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

/**@fn errprint_stats_infile
 * @brief print the source file name to stderr
 *
 * @param name[in] the name of the source file
**/
static void
errprint_stats_infile(const char *const restrict name)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fprintf(stderr, " in\t: %s\n", name);
	return;
}

/**@fn errprint_stats_outfile
 * @brief print the destination file name to stderr
 *
 * @param name[in] the name of the destination file
**/
static void
errprint_stats_outfile(const char *const restrict name)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fprintf(stderr, " out\t: %s\n", name);
	return;
}

/**@fn errprint_stats_format
 * @brief print file format stats to stderr
 *
 * @param fstat[in] the bloated file stats struct
 * @param mode encode or decode
**/
static void
errprint_stats_format(
	const struct FileStats *const restrict fstat,
	const enum ProgramMode mode
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	const char *inname, *outname;

	switch ( mode ){
	case MODE_ENCODE:
		inname	= decfmt_name(fstat->decfmt);
		outname	= encfmt_name(fstat->encfmt);
		break;
	case MODE_DECODE:
		inname	= encfmt_name(fstat->encfmt);
		outname	= decfmt_name(fstat->decfmt);
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
		fstat->inttype == INT_SIGNED ? 'i' : 'u'
	);
	(void) fprintf(stderr, "%"PRIu16"", fstat->samplebits);
	if ( fstat->samplebits > (u16) 8u ){
		(void) fprintf(stderr, "%s",
			fstat->endian == xENDIAN_LITTLE ? "le" : "be"
		);
	}
	//
	(void) fputs(" | ", stderr);
	(void) fprintf(stderr, "%"PRIu32" Hz", fstat->samplerate);
	//
	(void) fputs(" | ", stderr);
	(void) fprintf(stderr, "%"PRIu16"-ch", fstat->nchan);
/*
	(void) fputs(" (", stderr);
	(void) errprint_chanmask_wav(
		(uint) fstat->nchan, fstat->chanmask_wav
	);
	(void) fputc(')', stderr);
	//
*/
	(void) fputc('\n', stderr);
	return;

}

// this could be useful if TTA2 ever gets supported
#if 0
/**@fn errprint_stats_frame
 * @brief print stats about the size of a TTA frame to stderr
 *
 * @param fstat[in] the bloated file stats struct
**/
static void
errprint_stats_frame(const struct FileStats *const restrict fstat)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fputs(" frame\t: ", stderr);
	(void) fprintf(stderr, "%zu sample%s",
		fstat->framelen, fstat->framelen == (size_t) 1u ? "" : "s"
	);
	//
	(void) fputs("\t; ", stderr);
	errprint_size((size_t) (fstat->buflen * fstat->samplebytes));
	(void) fputc('B', stderr);
	//
	(void) fputs("\t, ", stderr);
	errprint_time(calc_time_pcm(fstat->buflen, fstat));
	//
	(void) fputc('\n', stderr);
	return;
}
#endif

/**@fn errprint_stats_pcm
 * @brief print stats about the unencoded PCM to stderr
 *
 * @param pcmtime length of the PCM in seconds
 * @param nframes number of TTA frames
 * @param nbytes_pcm size of the PCM
**/
static void
errprint_stats_pcm(
	const double pcmtime, const size_t nframes, const size_t nbytes_pcm
)
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

/**@fn errprint_stats_tta
 * @brief print stats about the encoded TTA to stderr
 *
 * @param pcmtime length of the PCM in seconds
 * @param nbytes_pcm size of the PCM
 * @param nbytes_tta size of the TTA
**/
static void
errprint_stats_tta(
	const double pcmtime, const size_t nbytes_pcm, const size_t nbytes_tta
)
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

/**@fn errprint_stats_codectime
 * @brief print stats about the time to en/de-code the source file to stderr
 *
 * @param pcmtime length of the PCM in seconds
 * @param codectime time it took to en/de-code the source file
 * @param nbytes_pcm size of the PCM
**/
static void
errprint_stats_codectime(
	const double pcmtime, const double codectime, const size_t nbytes_pcm
)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
{
	(void) fputs(" time   : ", stderr);
	errprint_time(codectime);
	//
	(void) fputs("\t; ", stderr);
	errprint_size((size_t) (((double) (8u * nbytes_pcm)) / codectime));
	(void) fputs("b/s", stderr);
	//
	(void) fputs("\t, ", stderr);
	(void) fprintf(stderr, "%.1f", pcmtime / codectime);
	(void) fputc('\n', stderr);
	//
	return;
}

//--------------------------------------------------------------------------//

/**@fn decfmt_name
 * @brief returns a string of the name of the decoded format
 *
 * @param fmt format id
 *
 * @return the name of the format
**/
/*@observer@*/
static CONST const char *
decfmt_name(const enum DecFormat fmt)
/*@*/
{
	/*@observer@*/
	const char *const name[] = DECFMT_NAME_ARRAY;
	return name[fmt];
}

/**@fn decfmt_name
 * @brief returns a string of the name of the encoded format
 *
 * @param fmt format id
 *
 * @return the name of the format
**/
/*@observer@*/
static CONST const char *
encfmt_name(const enum EncFormat fmt)
/*@*/
{
	/*@observer@*/
	const char *const name[] = xENCFMT_NAME_ARRAY;
	return name[fmt];
}

/**@fn calc_time_pcm
 * @brief calculates the length of PCM in seconds
 *
 * @param nsamples the number of PCM samples of 'nchan' channels
 * @param fstat[in] the bloated file stats struct
 *
 * @return the length in seconds
**/
static CONST double
calc_time_pcm(
	const size_t nsamples, const struct FileStats *const restrict fstat
)
/*@*/
{
	double size;
	size  = (double) nsamples;
	size /= (double) (fstat->samplerate * fstat->nchan);
	return size;
}

/**@fn errprint_size
 * @brief print a formatted size string to stderr
 *
 * @param size the size to format
**/
static void
errprint_size(const size_t size)
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

/**@fn errprint_time
 * @brief print a formatted time string to stderr
 *
 * @param sec number of seconds
**/
static void
errprint_time(const double sec)
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

// this could be useful if TTA2 ever gets supported
#if 0
/**@fn errprint_chanmask
 * @brief print a formatted channel mask string to stderr
 *
 * @param nchan number of audio channels
 * @param mask the channel mask
**/
static void
errprint_chanmask_wav(const uint nchan, const u32 mask)
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
				(void) fputs(chan_name[i], stderr);
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
#endif

// EOF ///////////////////////////////////////////////////////////////////////
