#ifndef TTA_MODES_THREADS_H
#define TTA_MODES_THREADS_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/threads.h                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      error checking or asserting threading function wrappers             //
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

typedef pthread_t		thread_p;
typedef sem_t			semaphore_p;
typedef pthread_spinlock_t	spinlock_p;

//////////////////////////////////////////////////////////////////////////////

/**@fn thread_create
 * @brief create a thread + error check
 *
 * @param thread[out] the thread
 * @param start_routine the function the thread will run
 * @param arg[in] the argument for the thread function
**/
INLINE void
thread_create(
	/*@out@*/ thread_p *const restrict thread,
	void *(*const start_routine) (void *),
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

/**@fn thread_join
 * @brief join a thread
 *
 * @param thread[in] the thread
**/
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

/**@fn semaphore_init
 * @brief initialize a semaphore + error check
 *
 * @param sem[out] the semaphore
 * @param value initial value for the semaphore
**/
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

/**@fn semaphore_destroy
 * @brief destroy a semaphore
 *
 * @param sem[in] the semaphore
**/
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

/**@fn semaphore_post
 * @brief increment a semaphore
 *
 * @param sem[in] the semaphore
**/
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

/**@fn semaphore_wait
 * @brief decrement a semaphore
 *
 * @param sem[in] the semaphore
**/
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

/**@fn spinlock_init
 * @brief initialize a spinlock + error check
 *
 * @param lock[out] the spinlock
**/
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

/**@fn spinlock_destroy
 * @brief destroy a spinlock
 *
 * @param lock[in] the spinlock
**/
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

/**@fn spinlock_lock
 * @brief lock a spinlock
 *
 * @param lock[in] the spinlock
**/
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

/**@fn spinlock_unlock
 * @brief unlock a spinlock
 *
 * @param lock[in] the spinlock
**/
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
