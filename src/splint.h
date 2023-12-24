#ifdef S_SPLINT_S
#ifndef TTA_SPLINT_H
#define TTA_SPLINT_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// splint.h                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      defines may not be technically correct (for every arch, or just in  //
// general), but this is just to shutup splint                              //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <stdint.h>
#include <time.h>

//////////////////////////////////////////////////////////////////////////////

typedef enum clockid_t	clockid_t;

struct timespec {
	time_t  tv_sec;
	long    tv_nsec;
};

//////////////////////////////////////////////////////////////////////////////

#ifndef UINT8_MAX
#define UINT8_MAX	((uint8_t) 0xFFu)
#endif

#ifndef UINT32_MAX
#define UINT32_MAX	((uint32_t) 0xFFFFFFFFu)
#endif

#ifndef UINT64_MAX
#define UINT64_MAX	((uint64_t) 0xFFFFFFFFFFFFFFFFu)
#endif

#ifndef SIZE_MAX
#define SIZE_MAX	((size_t) 0xFFFFFFFFFFFFFFFFu)
#endif

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC	((clockid_t) 1) /*bogus*/
#endif

//--------------------------------------------------------------------------//

#ifndef PRIX8
#define PRIX8	"hhX"
#endif

#ifndef PRIu16
#define PRIu16	"hu"
#endif

#ifndef PRIu32
#define PRIu32	"u"
#endif

#ifndef PRIX16
#define PRIX16	"hX"
#endif

//////////////////////////////////////////////////////////////////////////////

#undef res
/*@external@*/
extern int clock_gettime(clockid_t, /*@out@*/ struct timespec *res)
/*@modifies	res@*/
;

/*@external@*/
extern int fseeko(FILE *, off_t, int)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

/*@external@*/
extern off_t ftello(FILE *)
/*@globals	fileSystem@*/
/*@modifies	nothing@*/
;

/*@-type@*/
/*@external@*/
extern int ftruncate(int, off_t)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
/*@=type@*/
;

/*@-protoparammatch@*/
#undef rlim
/*@external@*/
extern int getrlimit(int, /*@out@*/ struct rlimit *rlim)
/*@globals	internalState@*/
/*@modifies	*rlim@*/
;
/*@=protoparammatch@*/

/*@temp@*/ /*@null@*/ /*@external@*/
extern void *memrchr(const void *, int, size_t)
/*@*/
;

/*@only@*/ /*@null@*/ /*@partial@*/ /*@external@*/
extern void *reallocarray(/*@only@*/ /*@null@*/ void *, size_t, size_t)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;

/*@external@*/
extern int setrlimit(int, /*@in@*/ struct rlimit *)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
#endif
