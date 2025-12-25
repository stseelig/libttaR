#ifndef H_TTA_MODES_THREADS_WIN32_H
#define H_TTA_MODES_THREADS_WIN32_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/threads.win32.h                                                    //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>

#include <windows.h>

#include "../common.h"
#include "../debug.h"

/* //////////////////////////////////////////////////////////////////////// */

#ifdef __GNUC__

#define	X_ATTRIBUTE_GNUC_STDCALL_ABI	stdcall

#else	/* !defined(__GNUC__) */

#define	X_ATTRIBUTE_GNUC_STDCALL_ABI	nil

#endif	/* __GNUC__ */

/* ======================================================================== */

#if X_HAS_ATTRIBUTE_GNUC(X_ATTRIBUTE_GNUC_STDCALL_ABI)
#define STDCALL_ABI		__attribute__((X_ATTRIBUTE_GNUC_STDCALL_ABI))
#else
#ifndef S_SPLINT_S
#error "compiler does not support the attribute 'stdcall'"
#else
#define STDCALL_ABI
#endif	/* S_SPLINT_S */
#endif	/* STDCALL_ABI */

/* //////////////////////////////////////////////////////////////////////// */

#define START_ROUTINE_ABI	STDCALL_ABI

typedef DWORD			start_routine_ret;
typedef HANDLE			thread_p;
typedef HANDLE			semaphore_p;
typedef CRITICAL_SECTION	spinlock_p;

/* //////////////////////////////////////////////////////////////////////// */

/**@see "threads.h" **/
INLINE void
thread_create(
	/*@out@*/ thread_p *const RESTRICT thread,
	start_routine_ret (*const start_routine) (void *) START_ROUTINE_ABI,
	/*@null@*/ void *const RESTRICT arg
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*thread
@*/
{
	*thread = CreateThread(
		NULL, 0, start_routine, arg, 0, NULL
	);

	if UNLIKELY ( *thread == NULL ){
		error_sys((int) GetLastError(), "CreateThread", NULL);
	}
	return;
}

/**@see "threads.h" **/
INLINE void
thread_join(const thread_p *const RESTRICT thread)
/*@globals	internalState@*/
/*@modifies	internalState@*/
{
	UNUSED const DWORD err = WaitForSingleObject(*thread, INFINITE);

	assert(err == 0);

	return;
}

/**@see "threads.h" **/
INLINE void
thread_detach_self(void)
/*@globals	internalState@*/
/*@modifies	internalState@*/
{
	UNUSED const BOOL err = CloseHandle(GetCurrentThread());

	assert(err != 0);

	return;
}

/* ======================================================================== */

/**@see "threads.h" **/
INLINE void
semaphore_init(
	/*@out@*/ semaphore_p *const RESTRICT sem, const unsigned int value
)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*sem
@*/
{
	*sem = CreateSemaphoreA(NULL, (LONG) value, LONG_MAX, NULL);

	if UNLIKELY ( *sem == NULL ){
		error_sys((int) GetLastError(), "CreateSemaphoreA", NULL);
	}
	return;
}

/**@see "threads.h" **/
INLINE void
semaphore_destroy(semaphore_p *const RESTRICT sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
{
	UNUSED const BOOL err = CloseHandle(*sem);

	assert(err != 0);

	return;
}

/**@see "threads.h" **/
ALWAYS_INLINE void
semaphore_post(semaphore_p *const RESTRICT sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
{
	UNUSED const BOOL err = ReleaseSemaphore(*sem, 1L, NULL);

	assert(err != 0);

	return;
}

/**@see "threads.h" **/
ALWAYS_INLINE void
semaphore_wait(semaphore_p *const RESTRICT sem)
/*@globals	internalState@*/
/*@modifies	internalState,
		*sem
@*/
{
	UNUSED const DWORD err = WaitForSingleObject(*sem, INFINITE);

	assert(err == 0);

	return;
}

//==========================================================================//

/**@see "threads.h" **/
INLINE void
spinlock_init(/*@out@*/ spinlock_p *const RESTRICT lock)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState,
		*lock
@*/
{
	InitializeCriticalSection(lock);

	return;
}

/**@see "threads.h" **/
INLINE void
spinlock_destroy(spinlock_p *const RESTRICT lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
{
	DeleteCriticalSection(lock);

	return;
}

/**@see "threads.h" **/
ALWAYS_INLINE void
spinlock_lock(spinlock_p *const RESTRICT lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
{
	EnterCriticalSection(lock);

	return;
}

/**@see "threads.h" **/
ALWAYS_INLINE void
spinlock_unlock(spinlock_p *const RESTRICT lock)
/*@globals	internalState@*/
/*@modifies	internalState,
		*lock
@*/
{
	LeaveCriticalSection(lock);

	return;
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_MODES_THREADS_WIN32_H */
