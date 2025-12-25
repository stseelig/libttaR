#ifndef H_TTA_MODES_PQUEUE_H
#define H_TTA_MODES_PQUEUE_H
/* ///////////////////////////////////////////////////////////////////////////
//                                                                          //
// modes/pqueue.h                                                           //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
// Copyright (C) 2023-2025, Shane Seelig                                    //
// SPDX-License-Identifier: GPL-3.0-or-later                                //
//                                                                          //
//////////////////////////////////////////////////////////////////////////////
//                                                                          //
//      These functions are for keeping track of the index for a set of     //
// parallel arrays that is used like a ring buffer. At one point, I was     //
// using an actual queue, but the id's were always sequentially ordered, so //
// I replaced the actual queue with these pseudo-queue functions, hence     //
// pqueue.                                                                  //
//                                                                          //
/////////////////////////////////////////////////////////////////////////// */

#include <assert.h>

#include "../common.h"

/* //////////////////////////////////////////////////////////////////////// */

struct PQueue {
	unsigned int	limit;
	unsigned int	next;
};

/* //////////////////////////////////////////////////////////////////////// */

/**@fn pqueue_init
 * @brief initializes a pseudo-queue struct
 *
 * @param q     - pointer to the queue
 * @param limit - maximum queue length
**/
ALWAYS_INLINE void
pqueue_init(
	/*@out@*/ struct PQueue *const RESTRICT q, const unsigned int limit
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

/**@fn pqueue_next
 * @brief get the next queue id
 *
 * @param curr  - current queue id
 * @param limit - maximum queue length
 *
 * @return next queue id
**/
CONST
ALWAYS_INLINE unsigned int
pqueue_next(const unsigned int curr, const unsigned int limit)
/*@*/
{
	return (curr + 1u < limit ? curr + 1u : 0);
}

/**@fn pqueue_pop
 * @brief pseudo-dequeue
 *
 * @param q - pointer to the queue
 *
 * @return next queue id
**/
ALWAYS_INLINE unsigned int
pqueue_pop(struct PQueue *const RESTRICT q)
/*@modifies	q->next@*/
{
	const unsigned int retval = q->next;

	q->next = pqueue_next(q->next, q->limit);

	return retval;
}

/* EOF //////////////////////////////////////////////////////////////////// */
#endif	/* H_TTA_MODES_PQUEUE_H */
