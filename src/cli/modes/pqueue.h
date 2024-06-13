#ifndef TTA_MODES_PQUEUE_H
#define TTA_MODES_PQUEUE_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/pqueue.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024, Shane Seelig                                         //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      replaced an actual queue with a pseudo-queue, because the id's are  //
// always sequential                                                        //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////

#include <assert.h>

#include "../../bits.h"

//////////////////////////////////////////////////////////////////////////////

struct PQueue {
	uint	limit;
	uint	next;
};

//////////////////////////////////////////////////////////////////////////////

/**@fn pqueue_init
 * @brief initializes a pseudo-queue struct
 *
 * @param q[out] the queue
 * @param limit the maximum queue length + 1u
**/
INLINE void
pqueue_init(
	/*@out@*/ register struct PQueue *const restrict q,
	register uint limit
)
/*@modifies	q->limit,
		q->next
@*/
{
	assert(limit != 0);

	q->limit = limit;
	q->next  = 0;
	return;
}

/**@fn pdequeue
 * @brief pseudo-dequeue
 *
 * @param q[in out] the queue
 *
 * @return the next queue id
**/
ALWAYS_INLINE uint
pdequeue(register struct PQueue *const restrict q)
/*@modifies	q->next@*/
{
	register const uint r = q->next;
	q->next = (r + 1u != q->limit ? r + 1u : 0);
	return r;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
