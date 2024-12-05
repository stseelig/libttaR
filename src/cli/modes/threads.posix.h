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

#define START_ROUTINE_ABI

typedef /*@null@*/ void *	start_routine_ret;
typedef pthread_t		thread_p;
typedef sem_t			semaphore_p;
typedef pthread_spinlock_t	spinlock_p;

//////////////////////////////////////////////////////////////////////////////

/// @see "threads.h"
INLINE void
thread_create(
	/*@out@*/ thread_p *const restrict thread,
	start_routine_ret (*const start_routine) (void *) START_ROUTINE_ABI,
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
	const int err = pthread_create(thread, NULL, start_routine, arg);
	if UNLIKELY ( err != 0 ){
		error_sys(err, "pthread_create", NULL);
	}
	return;
}

/// @see "threads.h"
INLINE void
thread_join(const thread_p *const restrict thread)
/*@globals	internalState@*/
/*@modifies	internalState@*/
{
	UNUSED const int err = pthread_join(*thread, NULL);
	assert(err == 0);
	return;
}

/// @see "threads.h"
INLINE void
thread_detach(const thread_p *const restrict thread)
/*@globals	internalState@*/
/*@modifies	internalState@*/
{
	UNUSED const int err = pthread_detach(*thread);;
	assert(err == 0);
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
	const int err = sem_init(sem, 0, value);
	if UNLIKELY ( err != 0 ){
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
	UNUSED const int err = sem_destroy(sem);
	assert(err == 0);
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
	UNUSED const int err = sem_post(sem);
	assert(err == 0);
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
	int err;
try_again:
	err = sem_wait(sem);
	if UNLIKELY ( err != 0 ){
		if ( errno == EINTR ){
			goto try_again;
		}
	}
	assert(err == 0);
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
	const int err = pthread_spin_init(lock, PTHREAD_PROCESS_PRIVATE);
	if UNLIKELY ( err != 0 ){
		error_sys(err, "pthread_spin_init", NULL);
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
	UNUSED const int err = pthread_spin_destroy(lock);
	assert(err == 0);
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
	UNUSED const int err = pthread_spin_lock(lock);
	assert(err == 0);
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
	UNUSED const int err = pthread_spin_unlock(lock);
	assert(err == 0);
	return;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
