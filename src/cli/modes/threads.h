#ifndef H_TTA_MODES_THREADS_H
#define H_TTA_MODES_THREADS_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/threads.h                                                          //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      error checking or asserting threading function wrappers             //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "../common.h"

/* //////////////////////////////////////////////////////////////////////// */

#if 0	/* system-type */

#elif defined(__unix__) || defined(S_SPLINT_S)
#include "threads.posix.h"

#elif defined(__WIN32__)
#include "threads.win32.h"

#else
#error "unsupported system"

#endif	/* system-type */

/* //////////////////////////////////////////////////////////////////////// */

/*@-redecl@*/

/* ------------------------------------------------------------------------ */

#undef thread
/**@fn thread_create
 * @brief create a thread + error check
 *
 * @param thread        - pointer to the thread object
 * @param start_routine - function the thread will run
 * @param arg           - argument for the thread function
**/
INLINE void thread_create(
	/*@out@*/ thread_p *RESTRICT thread,
	start_routine_ret (*) (void *) START_ROUTINE_ABI,
	/*@null@*/ void *RESTRICT
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
 * @param thread - pointer to the thread object
**/
INLINE void thread_join(const thread_p *RESTRICT)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;

/**@fn thread_detach_self
 * @brief the calling thread detaches itself
**/
INLINE void thread_detach_self(void)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;

/* ------------------------------------------------------------------------ */

#undef sem
/**@fn semaphore_init
 * @brief initialize a semaphore + error check
 *
 * @param sem   - pointer to the semaphore
 * @param value - initial value for the semaphore
**/
INLINE void semaphore_init(/*@out@*/ semaphore_p *RESTRICT sem, unsigned int)
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
 * @param sem - pointer to the semaphore
**/
INLINE void semaphore_destroy(semaphore_p *RESTRICT sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;

#undef sem
/**@fn semaphore_post
 * @brief increment a semaphore
 *
 * @param sem - pointer to the semaphore
**/
ALWAYS_INLINE void semaphore_post(semaphore_p *RESTRICT sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;

#undef sem
/**@fn semaphore_wait
 * @brief decrement a semaphore
 *
 * @param sem - pointer to the semaphore
**/
ALWAYS_INLINE void semaphore_wait(semaphore_p *RESTRICT sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
;

/* ------------------------------------------------------------------------ */

#undef lock
/**@fn spinlock_init
 * @brief initialize a spinlock + error check
 *
 * @param lock - pointer to the spinlock
**/
INLINE void spinlock_init(/*@out@*/ spinlock_p *RESTRICT lock)
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
 * @param lock - pointer to the spinlock
**/
INLINE void spinlock_destroy(spinlock_p *RESTRICT lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;

#undef lock
/**@fn spinlock_lock
 * @brief lock a spinlock
 *
 * @param lock - pointer to the spinlock
**/
ALWAYS_INLINE void spinlock_lock(spinlock_p *RESTRICT lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;

#undef lock
/**@fn spinlock_unlock
 * @brief unlock a spinlock
 *
 * @param lock - pointer to the spinlock
**/
ALWAYS_INLINE void spinlock_unlock(spinlock_p *RESTRICT lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
;

/* ------------------------------------------------------------------------ */

/*@=redecl@*/

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_MODES_THREADS_H */
