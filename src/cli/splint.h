#ifndef H_TTA_SPLINT_H
#define H_TTA_SPLINT_H
#ifdef S_SPLINT_S
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// splint.h                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      defines may not be technically correct (for every arch, or just in  //
// general), but this is just to shutup splint                              //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <stdint.h>
#include <time.h>

/* //////////////////////////////////////////////////////////////////////// */

/*@-namechecks@*/
#define __off_t		off_t
#define __off64_t 	off_t
/*@=namechecks@*/

/*@-redef@*/
typedef enum clockid_t	clockid_t;
/*@=redef@*/

/*@-redef@*/ /*@-matchfields@*/
struct timespec {
	time_t  tv_sec;
	long    tv_nsec;
};
/*@=redef@*/ /*@=matchfields@*/

/* these are bogus */
/*@-redef@*/
typedef int	pthread_t;
typedef int	pthread_attr_t;
typedef int	pthread_spinlock_t;
typedef int	sem_t;
/*@=redef@*/

/* //////////////////////////////////////////////////////////////////////// */

#ifndef UINT8_MAX
#define UINT8_MAX	((uint8_t) 0xFFu)
#endif

#ifndef UINT16_MAX
#define UINT16_MAX	((uint16_t) 0xFFFFu)
#endif

#ifndef UINT32_MAX
#define UINT32_MAX	((uint32_t) 0xFFFFFFFFu)
#endif

#ifndef UINT64_MAX
#define UINT64_MAX	((uint64_t) 0xFFFFFFFFFFFFFFFFu)
#endif

#ifndef UINTMAX_MAX
#define UINTMAX_MAX	((uintmax_t) 0xFFFFFFFFFFFFFFFFu)
#endif

#ifndef SIZE_MAX
#define SIZE_MAX	((size_t) 0xFFFFFFFFFFFFFFFFu)
#endif

/* ------------------------------------------------------------------------ */

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC	((clockid_t) 1)	/* bogus */
#endif

/* ------------------------------------------------------------------------ */

#ifndef PRId8
#define PRId8	"hhd"
#endif

#ifndef PRIX8
#define PRIX8	"hhX"
#endif

#ifndef PRIX16
#define PRIX16	"hX"
#endif

#ifndef PRIu16
#define PRIu16	"hu"
#endif

#ifndef PRIu32
#define PRIu32	"u"
#endif

/* //////////////////////////////////////////////////////////////////////// */

/*@external@*/ /*@unused@*/
extern long long atoll(const char *) /*@*/;

/*@-incondefs@*/
/*@external@*/ /*@unused@*/
/*@only@*/ /*@null@*/ /*@in@*/
extern void *calloc(size_t, size_t)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;
/*@=incondefs@*/

#undef res
/*@external@*/ /*@unused@*/
extern int clock_gettime(clockid_t, /*@out@*/ struct timespec *res)
/*@modifies	*res@*/
;

#undef filehandle
/*@external@*/ /*@unused@*/
extern void flockfile(FILE *filehandle)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		filehandle
@*/
;

/*@external@*/ /*@unused@*/
extern int fseeko(FILE *, off_t, int)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
;

/*@external@*/ /*@unused@*/
extern off_t ftello(FILE *)
/*@globals	fileSystem@*/
/*@modifies	nothing@*/
;

/*@-type@*/
/*@external@*/ /*@unused@*/
extern int ftruncate(int, off_t)
/*@globals	fileSystem@*/
/*@modifies	fileSystem@*/
/*@=type@*/
;

#undef filehandle
/*@external@*/ /*@unused@*/
extern void funlockfile(FILE *filehandle)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		filehandle
@*/
;

/*@-incondefs@*/
#undef oldact
/*@external@*/ /*@unused@*/
extern int sigaction(
	int, /*@null@*/ const struct sigaction *,
	/*@out@*/ /*@null@*/ struct sigaction *oldact
)
/*@globals	internalState@*/
/*@modifies	internalState,
		*oldact
@*/
;
/*@=incondefs@*/

/*@-protoparammatch@*/
#undef rlim
/*@external@*/ /*@unused@*/
extern int getrlimit(int, /*@out@*/ struct rlimit *rlim)
/*@globals	internalState@*/
/*@modifies	*rlim@*/
;
/*@=protoparammatch@*/

/*@-incondefs@*/
/*@external@*/ /*@unused@*/
/*@only@*/ /*@null@*/ /*@out@*/
extern void *malloc(size_t)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;
/*@=incondefs@*/

/*@-incondefs@*/
#undef ptr
/*@external@*/ /*@unused@*/
/*@only@*/ /*@null@*/ /*@partial@*/
extern void *realloc(/*@only@*/ /*@null@*/ /*@out@*/ void *ptr, size_t)
/*@globals	internalState@*/
/*@modifies	internalState,
		*ptr
@*/
;
/*@=incondefs@*/

/*@external@*/ /*@unused@*/
extern int setrlimit(int, /*@in@*/ struct rlimit *)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;

/*@-incondefs@*/
#undef str
/*@external@*/ /*@unused@*/
/*@printflike@*/
extern int snprintf(char *str, size_t, const char *, ...)
/*@modifies	*str@*/
;
/*@=incondefs@*/

#undef buf
/*@external@*/ /*@unused@*/
extern int strerror_r(int, /*@out@*/ char *buf, size_t)
/*@modifies	*buf@*/
;

/*@external@*/ /*@unused@*/
/*@observer@*/
extern char *strsignal(int sig) /*@*/;

/* ======================================================================== */

/*@-protoparammatch@*/
#undef thread
/*@external@*/ /*@unused@*/
extern int pthread_create(
	/*@out@*/ pthread_t *thread, /*@null@*/ const pthread_attr_t *,
	void *(*) (void *), /*@null@*/ void *
)
/*@globals	internalState@*/
/*@modifies	internalState,
		*thread
@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
#undef retval
/*@external@*/ /*@unused@*/
extern int pthread_join(pthread_t, /*@null@*/ void **retval)
/*@globals	internalState@*/
/*@modifies	internalState,
		*retval
@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
/*@external@*/ /*@unused@*/
extern int pthread_detach(pthread_t)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
/*@external@*/ /*@unused@*/
extern pthread_t pthread_self(void)
/*@globals	internalState@*/
/*@modifies	nothing@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
#undef lock
/*@external@*/ /*@unused@*/
extern int pthread_spin_destroy(pthread_spinlock_t *lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
#undef lock
/*@external@*/ /*@unused@*/
extern int pthread_spin_init(/*@out@*/ pthread_spinlock_t *lock, int)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
#undef lock
/*@external@*/ /*@unused@*/
extern int pthread_spin_lock(pthread_spinlock_t *lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
#undef lock
/*@external@*/ /*@unused@*/
extern int pthread_spin_unlock(pthread_spinlock_t *lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
#undef sem
/*@external@*/ /*@unused@*/
extern int sem_destroy(sem_t *sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
#undef sem
/*@external@*/ /*@unused@*/
extern int sem_init(/*@out@*/ sem_t *sem, int, unsigned int)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
#undef sem
/*@external@*/ /*@unused@*/
extern int sem_post(sem_t *sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;
/*@=protoparammatch@*/

/*@-protoparammatch@*/
#undef sem
/*@external@*/ /*@unused@*/
extern int sem_wait(sem_t *sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;
/*@=protoparammatch@*/

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* S_SPLINT_S */
#endif	/* H_TTA_SPLINT_H */
