/*
 * General implementation of semaphores.
 *
 *--------------------------------------------------------------------
 * Adapted from code for CS24 by Jason Hickey.
 * Copyright (C) 2003-2010, Caltech.  All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <assert.h>

#include "sthread.h"
#include "queue.h"
#include "semaphore.h"

/*
 * The semaphore data structure contains a non-negative integer i.
 * If i is zero, then no other thread can use the shared resource.
 * If i is non-zero, the semaphore will signal and ask another
 * thread to run.
 */
struct _semaphore {
    int i;
    /* In order to be fair, we maintain the blocked threads in a queue, which
     * ensures all the blocked threads would eventually acquire the semaphore.
     */
    Queue blocked_queue;
};

/************************************************************************
 * Top-level semaphore implementation.
 */

/*
 * Allocate a new semaphore.  The initial value of the semaphore is
 * specified by the argument.
 */
Semaphore *new_semaphore(int init) {
    Semaphore *semp = NULL;

    /* Allocate and initialize a semaphore data struct. */
    semp = (Semaphore *) malloc(sizeof(Semaphore));
    if (semp == NULL) {
        /* Handles malloc failure */
        fprintf(stderr, "Can't allocate a Semaphore struct.\n");
        exit(1);    
    }
    semp->i = init;
    return semp;
}

/*
 * Decrement the semaphore.
 * This operation must be atomic, and it blocks iff the semaphore is zero.
 */
void semaphore_wait(Semaphore *semp) {

    /* Lock this process in order to be atomic: if this process is interupted,
     * the semaphore would be corrupted.
     */
    __sthread_lock();

    /* Ensure the semaphore invariant */
    assert(semp->i >= 0);
    
    /* If i is zero, block the current thread; otherwise decrement i. 
     * Need to wait until some thread increments the semaphore .
     */
    while (semp->i == 0) {
        /* put the current thread into blocked_queue */
        queue_append(&(semp->blocked_queue), sthread_current());

        sthread_block();

        /* Need to relock since the scheduler would unlock when sthread_block
         * is returned. 
         */
        __sthread_lock();
    }
    
    semp->i--;

    /* Unlock this process since it is finished. */
    __sthread_unlock();
}

/*
 * Increment the semaphore.
 * This operation must be atomic.
 */
void semaphore_signal(Semaphore *semp) {

    /* Lock this process in order to be atomic: if this process is interupted,
     * the semaphore would be corrupted.
     */
    __sthread_lock();
    
    /* Ensure the semaphore invariant */
    assert(semp->i >= 0);

    /* Increment i */
    semp->i++;

    /* If the there are blocked thread in the queue, unblock it. */
    if (!queue_empty(&(semp->blocked_queue))) {
        sthread_unblock(queue_take(&(semp->blocked_queue)));
    }

    /* Unlock this process since it is finished. */
    __sthread_unlock();
}

