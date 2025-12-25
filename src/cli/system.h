#ifndef H_TTA_SYSTEM_H
#define H_TTA_SYSTEM_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// system.h                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      not all POSIX functions are wrapped, just those not automatically   //
// converted by MinGW                                                       //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include "./common.h"

/* //////////////////////////////////////////////////////////////////////// */

#if 0	/* system-type */

#elif defined(__unix__) || defined(S_SPLINT_S)
#include "system.posix.h"

#elif defined(__WIN32__)
#include "system.win32.h"

#else
#error "unsupported system"

#endif	/* system-type */

/* //////////////////////////////////////////////////////////////////////// */

/*@-redecl@*/

/* ------------------------------------------------------------------------ */

/**@fn signals_setup
 * @brief handles all of the programs signals setup
 * @note this program only uses basic signaling for temp file cleanup
**/
INLINE void signals_setup(void)
/*@globals	internalState@*/
/*@modifies	internalState@*/
;

#undef dest
/**@fn timestamp_get
 * @brief gets a timestamp in seconds/nanoseconds (clock_gettime wrapper)
 *
 * @param dest -timestamp destination
**/
ALWAYS_INLINE void timestamp_get(/*@out@*/ timestamp_p *RESTRICT dest)
/*@globals	internalState@*/
;

#undef start
#undef finish
/**@fn timestamp_diff
 * @brief diffs two timestamps
 *
 * @param start  - start time
 * @param finish - finish time
 *
 * @return elapsed time in seconds from start to finish
**/
INLINE PURE double timestamp_diff(
	const timestamp_p *RESTRICT start, const timestamp_p *RESTRICT finish
)
/*@*/
;

#undef filehandle
/**@fn file_lock
 * @brief locks a file (flockfile wrapper)
 *
 * @param filehandle - FILE pointer
**/
ALWAYS_INLINE void file_lock(FILE *RESTRICT filehandle)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		filehandle
@*/
;

#undef filehandle
/**@fn file_unlock
 * @brief unlocks a file (funlockfile wrapper)
 *
 * @param filehandle - FILE pointer
**/
ALWAYS_INLINE void file_unlock(FILE *RESTRICT filehandle)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		filehandle
@*/
;

/**@fn fdlimit_check
 * @brief attempt to increase the open-file limit with error checking
**/
INLINE void fdlimit_check(void)
/*@globals	fileSystem,
		internalState
@*/
/*@modifies	fileSystem,
		internalState
@*/
;

/**@fn get_nprocessors_onln
 * @brief sysconf(_SC_NPROCESSORS_ONLN) wrapper
 *
 * @return number of online processors
**/
INLINE unsigned int get_nprocessors_onln(void)
/*@globals	internalState*/
;

#undef errnum
#undef buf
#undef buflen
/**@fn strerror_ts
 * @brief thread-safe strerror (strerror_r wrapper)
 *
 * @param errnum - error number
 * @param buf    - buffer to write
 * @param buflen - length of the buffer
 *
 * @return buf
**/
/*@temp@*/
INLINE char *strerror_ts(
	int errnum, /*@out@*/ /*@returned@*/ char *RESTRICT buf, size_t buflen
)
/*@modifies	*buf@*/
;

/* ------------------------------------------------------------------------ */

/*@=redecl@*/

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_SYSTEM_H */
