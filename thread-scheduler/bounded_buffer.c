/*
 * Define a bounded buffer containing records that describe the
 * results in a producer thread.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>

#include "sthread.h"
#include "bounded_buffer.h"
#include "semaphore.h"

/*
 * The bounded buffer data.
 */
struct _bounded_buffer {
    /* The maximum number of elements in the buffer */
    int length;

    /* The index of the first element in the buffer */
    int first;

    /* The number of elements currently stored in the buffer */
    int count;

    /* The values in the buffer */
    BufferElem *buffer;

    /* This semaphore is in charge of how many element we can add to the buffer
     * until it is full.
     */
    Semaphore *to_full_semp;

    /* This semaphore is in charge of how many element we can take from the 
     * buffer until it is empty.
     */
    Semaphore *to_empty_semp;

    /* This semaphore ensures only one thread can call add() at a time. */
    Semaphore *add_in_use;

    /* This semaphore ensures only one thread can call take() at a time. */
    Semaphore *take_in_use;
};


#define EMPTY -1


/*
 * Allocate a new bounded buffer.
 */
BoundedBuffer *new_bounded_buffer(int length) {
    BoundedBuffer *bufp;
    BufferElem *buffer;

    Semaphore *to_full_semp;
    Semaphore *to_empty_semp;
    Semaphore *add_in_use;
    Semaphore *take_in_use;

    int i;

    /* Allocate the buffer */
    buffer = (BufferElem *) malloc(length * sizeof(BufferElem));
    bufp = (BoundedBuffer *) malloc(sizeof(BoundedBuffer));

    if (buffer == 0 || bufp == 0) {
        fprintf(stderr, "new_bounded_buffer: out of memory\n");
        exit(1);
    }

    /* Initialize */

    memset(bufp, 0, sizeof(BoundedBuffer));

    for (i = 0; i != length; i++) {
        buffer[i].id = EMPTY;
        buffer[i].arg = EMPTY;
        buffer[i].val = EMPTY;
    }

    /* Initialize semaphores. 
     * to_full_semp and add_in_use is decremented when add() is called;
     * to_empty_semp is incremented after add() returns.
     * to_empty_semp and take_in_use is decremented when take() is called;
     * to_full_semp is incremented after take() returns.
     */
    to_full_semp = new_semaphore(length);
    to_empty_semp = new_semaphore(0);
    add_in_use = new_semaphore(1);
    take_in_use = new_semaphore(1);

    bufp->length = length;
    bufp->buffer = buffer;

    bufp->to_full_semp = to_full_semp;
    bufp->to_empty_semp = to_empty_semp;
    bufp->add_in_use = add_in_use;
    bufp->take_in_use = take_in_use;

    return bufp;
}

/*
 * Add an integer to the buffer.  Yield control to another
 * thread if the buffer is full.
 */
void bounded_buffer_add(BoundedBuffer *bufp, const BufferElem *elem) {

    /* add_in_use: initialized with i = 1, so it
     * blocks the whole add function to ensure it's atomic. */
    semaphore_wait(bufp->add_in_use);

    /* to_full_semp: initialized with i = buffer length.
     * Decremented when add is called, if to_full_semp == 0,
     * we know the buffer is full so adding threads are blocked.
     */
    semaphore_wait(bufp->to_full_semp);

    /* Now the buffer has space.  Copy the element data over. */
    int idx = (bufp->first + bufp->count) % bufp->length;
    bufp->buffer[idx].id  = elem->id;
    bufp->buffer[idx].arg = elem->arg;
    bufp->buffer[idx].val = elem->val;

    bufp->count = bufp->count + 1;

    /* Increment to_empty_semp, because we successfully added an element,
     * and made the buffer further away from being empty.
     */
    semaphore_signal(bufp->to_empty_semp);

    /* release the function block for other threads. */
    semaphore_signal(bufp->add_in_use);
}

/*
 * Get an integer from the buffer.  Yield control to another
 * thread if the buffer is empty.
 */
void bounded_buffer_take(BoundedBuffer *bufp, BufferElem *elem) {

    /* take_in_use: initialized with i = 1, so it
     * blocks the whole add function to ensure it's atomic. */
    semaphore_wait(bufp->take_in_use);

    /* to_empty_semp: initialized with i = 0.
     * Decremented when take is called, if to_empty_semp == 0,
     * we know the buffer is empty so taking threads are blocked.
     */
    semaphore_wait(bufp->to_empty_semp);

    /* Copy the element from the buffer, and clear the record */
    elem->id  = bufp->buffer[bufp->first].id;
    elem->arg = bufp->buffer[bufp->first].arg;
    elem->val = bufp->buffer[bufp->first].val;

    bufp->buffer[bufp->first].id  = -1;
    bufp->buffer[bufp->first].arg = -1;
    bufp->buffer[bufp->first].val = -1;

    bufp->count = bufp->count - 1;
    bufp->first = (bufp->first + 1) % bufp->length;

    /* Increment to_full_semp, because we successfully removed an element,
     * and made the buffer further away from being full.
     */
    semaphore_signal(bufp->to_full_semp);

    /* release the function block for other threads. */
    semaphore_signal(bufp->take_in_use);
}

