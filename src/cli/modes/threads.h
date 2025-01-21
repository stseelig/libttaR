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

#include "../../bits.h"

#if defined(__unix__)
#include "threads.posix.h"
#elif defined(__WIN32__)
#include "threads.win32.h"
#else
#error "unsupported system"
#endif

//////////////////////////////////////////////////////////////////////////////

#undef thread
/**@fn thread_create
 * @brief create a thread + error check
 *
 * @param thread[out] the thread
 * @param start_routine the function the thread will run
 * @param arg[in] the argument for the thread function
**/
INLINE void thread_create(
	/*@out@*/ thread_p *restrict thread,
	start_routine_ret (*) (void *) START_ROUTINE_ABI,
	/*@null@*/ void *restrict
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*thread
@*/
;

/**@fn thread_join
 * @brief join a thread
 *
 * @param thread[in] the thread
**/
INLINE void thread_join(const thread_p *restrict)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;

/**@fn thread_detach_self
 * @brief the calling thread detaches itself
 *
 * @param thread[in] the thread
**/
INLINE void thread_detach_self(void)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;

//==========================================================================//

#undef sem
/**@fn semaphore_init
 * @brief initialize a semaphore + error check
 *
 * @param sem[out] the semaphore
 * @param value initial value for the semaphore
**/
INLINE void semaphore_init(/*@out@*/ semaphore_p *restrict sem, uint)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*sem
@*/
;

#undef sem
/**@fn semaphore_destroy
 * @brief destroy a semaphore
 *
 * @param sem[in out] the semaphore
**/
INLINE void semaphore_destroy(semaphore_p *restrict sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;

#undef sem
/**@fn semaphore_post
 * @brief increment a semaphore
 *
 * @param sem[in out] the semaphore
**/
ALWAYS_INLINE void semaphore_post(semaphore_p *restrict sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;

#undef sem
/**@fn semaphore_wait
 * @brief decrement a semaphore
 *
 * @param sem[in out] the semaphore
**/
ALWAYS_INLINE void semaphore_wait(semaphore_p *restrict sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;

//==========================================================================//

#undef lock
/**@fn spinlock_init
 * @brief initialize a spinlock + error check
 *
 * @param lock[out] the spinlock
**/
INLINE void spinlock_init(/*@out@*/ spinlock_p *restrict lock)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*lock
@*/
;

#undef lock
/**@fn spinlock_destroy
 * @brief destroy a spinlock
 *
 * @param lock[in out] the spinlock
**/
INLINE void spinlock_destroy(spinlock_p *restrict lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;

#undef lock
/**@fn spinlock_lock
 * @brief lock a spinlock
 *
 * @param lock[in out] the spinlock
**/
ALWAYS_INLINE void spinlock_lock(spinlock_p *restrict lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;

#undef lock
/**@fn spinlock_unlock
 * @brief unlock a spinlock
 *
 * @param lock[in out] the spinlock
**/
ALWAYS_INLINE void spinlock_unlock(spinlock_p *restrict lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
