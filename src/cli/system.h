#ifndef TTA_SYSTEM_H
#define TTA_SYSTEM_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// system.h                                                                 //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      not all POSIX functions are wrapped, just those not automatically   //
// converted by MinGW                                                       //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include "../bits.h"

#if defined(__unix__)
#include "system.posix.h"
#elif defined(__WIN32__)
#include "system.win32.h"
#else
#error "unsupported system"
#endif

//////////////////////////////////////////////////////////////////////////////

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
 * @param dest[out] timestamp destination
**/
ALWAYS_INLINE void timestamp_get(/*@out@*/ timestamp_p *restrict dest)
/*@globals	internalState@*/
;

#undef start
#undef finish
/**@fn timestamp_diff
 * @brief diffs two timestamps
 *
 * @param start[in] start time
 * @param finish[in] finish time
 *
 * @return the elapsed time in seconds from start to finish
**/
INLINE PURE double timestamp_diff(
	const timestamp_p *restrict start, const timestamp_p *restrict finish
)
/*@*/
;

#undef filehandle
/**@fn file_lock
 * @brief locks a file (flockfile wrapper)
 *
 * @param filehandle[in out] the file
**/
ALWAYS_INLINE void file_lock(FILE *restrict filehandle)
/*@globals	fileSystem@*/
/*@modifies	fileSystem,
		filehandle
@*/
;

#undef filehandle
/**@fn file_unlock
 * @brief unlocks a file (funlockfile wrapper)
 *
 * @param filehandle[in out] the file
**/
ALWAYS_INLINE void file_unlock(FILE *restrict filehandle)
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
 * @return the number of online processors
**/
INLINE uint get_nprocessors_onln(void)
/*@globals	internalState*/
;

#undef errnum
#undef buf
#undef buflen
/**@fn strerror_ts
 * @brief thread-safe strerror (strerror_r wrapper)
 *
 * @param errnum the error number
 * @param buf[out] the buffer to write
 * @param buflen length of the buffer
 *
 * @return buf
**/
/*@temp@*/
INLINE char *strerror_ts(
	int errnum, /*@out@*/ /*@returned@*/ char *restrict buf, size_t buflen
)
/*@modifies	*buf@*/
;

// EOF ///////////////////////////////////////////////////////////////////////
#endif
