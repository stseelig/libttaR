#ifndef TTA_MODES_PQUEUE_H
#define TTA_MODES_PQUEUE_H
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/pqueue.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2024-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      these functions are for keeping track of the index for a set of     //
// parallel arrays that is used like a ring buffer. at one point, I was     //
// using an actual queue, but the id's were always sequentially ordered, so //
// I replaced the actual queue with these pseudo-queue functions, hence     //
// pqueue                                                                   //
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
 * @param limit the maximum queue length
 *
 * @pre limit != 0
**/
INLINE void
pqueue_init(/*@out@*/ struct PQueue *const restrict q, const uint limit)
/*@modifies	q->limit,
		q->next
@*/
{
	assert(limit != 0);

	q->limit = limit;
	q->next  = 0;
	return;
}

/**@fn pqueue_next
 * @brief get the next queue id
 *
 * @param curr the current queue id
 * @param limit the maximum queue length
 *
 * @return the next queue id
**/
ALWAYS_INLINE CONST uint
pqueue_next(const uint curr, const uint limit)
/*@*/
{
	return (curr + 1u < limit ? curr + 1u : 0);
}

/**@fn pqueue_pop
 * @brief pseudo-dequeue
 *
 * @param q[in out] the queue
 *
 * @return the next queue id
 *
 * @note guarded by a spinlock
**/
ALWAYS_INLINE uint
pqueue_pop(struct PQueue *const restrict q)
/*@modifies	q->next@*/
{
	const uint retval = q->next;
	q->next = pqueue_next(q->next, q->limit);
	return retval;
}

// EOF ///////////////////////////////////////////////////////////////////////
#endif
