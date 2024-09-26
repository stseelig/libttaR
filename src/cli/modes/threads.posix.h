#ifndef TTA_MODES_THREADS_POSIX_H
#define TTA_MODES_THREADS_POSIX_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/threads.posix.h                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>
#include <errno.h>

#include <pthread.h>
#include <semaphore.h>

#include "../../bits.h"
#include "../../splint.h"

#include "../debug.h"

//////////////////////////////////////////////////////////////////////////////

typedef void *			start_routine_ret;
typedef pthread_t		thread_p;
typedef sem_t			semaphore_p;
typedef pthread_spinlock_t	spinlock_p;

//////////////////////////////////////////////////////////////////////////////

/// @see "threads.h"
INLINE void
thread_create(
	/*@out@*/ thread_p *const restrict thread,
	start_routine_ret (*const start_routine) (void *),
	/*@null@*/ void *const restrict arg
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*thread
@*/
{
	const int rv = pthread_create(thread, NULL, start_routine, arg);
	if UNLIKELY ( rv != 0 ){
		error_sys(rv, "pthread_create", NULL);
	}
	return;
}

/// @see "threads.h"
INLINE void
thread_join(const thread_p *const restrict thread)
/*@globals	internalState@*/
/*@modifies	internalState@*/
{
	const int rv = pthread_join(*thread, NULL);
	assert(rv == 0);
#ifdef NDEBUG
	(void) rv;
#endif
	return;
}

//==========================================================================//

/// @see "threads.h"
INLINE void
semaphore_init(/*@out@*/ semaphore_p *const restrict sem, const uint value)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*sem
@*/
{
	const int rv = sem_init(sem, 0, value);
	if UNLIKELY ( rv != 0 ){
		error_sys(errno, "sem_init", NULL);
	}
	return;
}

/// @see "threads.h"
INLINE void
semaphore_destroy(semaphore_p *const restrict sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
{
	const int rv = sem_destroy(sem);
	assert(rv == 0);
#ifdef NDEBUG
	(void) rv;
#endif
	return;
}

/// @see "threads.h"
ALWAYS_INLINE void
semaphore_post(semaphore_p *const restrict sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
{
	const int rv = sem_post(sem);
	assert(rv == 0);
#ifdef NDEBUG
	(void) rv;
#endif
	return;
}

/// @see "threads.h"
ALWAYS_INLINE void
semaphore_wait(semaphore_p *const restrict sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
{
	const int rv = sem_wait(sem);
	assert(rv == 0);
#ifdef NDEBUG
	(void) rv;
#endif
	return;
}

//==========================================================================//

/// @see "threads.h"
INLINE void
spinlock_init(/*@out@*/ spinlock_p *const restrict lock)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*lock
@*/
{
	const int rv = pthread_spin_init(lock, PTHREAD_PROCESS_PRIVATE);
	if UNLIKELY ( rv != 0 ){
		error_sys(rv, "pthread_spin_init", NULL);
	}
	return;
}

/// @see "threads.h"
INLINE void
spinlock_destroy(spinlock_p *const restrict lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
{
	const int rv = pthread_spin_destroy(lock);
	assert(rv == 0);
#ifdef NDEBUG
	(void) rv;
#endif
	return;
}

/// @see "threads.h"
ALWAYS_INLINE void
spinlock_lock(spinlock_p *const restrict lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
{
	const int rv = pthread_spin_lock(lock);
	assert(rv == 0);
#ifdef NDEBUG
	(void) rv;
#endif
	return;
}

/// @see "threads.h"
ALWAYS_INLINE void
spinlock_unlock(spinlock_p *const restrict lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
{
	const int rv = pthread_spin_unlock(lock);
	assert(rv == 0);
#ifdef NDEBUG
	(void) rv;
#endif
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
