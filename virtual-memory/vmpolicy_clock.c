/*============================================================================
 * Implementation of the CLOCK page replacement policy.
 *
 * We don't mind if paging policies use malloc() and free(), just because it
 * keeps things simpler.  In real life, the pager would use the kernel memory
 * allocator to manage data structures, and it would endeavor to allocate and
 * release as little memory as possible to avoid making the kernel slow and
 * prone to memory fragmentation issues.
 *
 * The CLOCK policy uses a circular queue in the memory to store virtual pages,
 * and there is a "clock hand" pointing to the page for eviction consideration.
 * When a page is to be evicted, CLOCK does:
 * (a) If the page under clock hand has a set accessed bit, it is cleared and
 *     the clock hand goes on.
 * (b) If the page has a clear accessed bit, it is chosen for eviction.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmpolicy.h"
#include "virtualmem.h"


/*============================================================================
 * Circular Queue Data Structure
 *
 * This data structure records all pages that are currently loaded in the
 * virtual memory in a circular queue, and there is a clock hand pointing
 * to the page to be considered for eviction. 
 * We use an array to store the queue.
 */

typedef struct c_queue_t {
    /* The maximum number of pages that can be resident in memory at once. */
    int max_resident;

    /* Pointing to the page to be considered for eviction. */
    int clock_hand;

    /* This is the array of pages that are actually loaded. */
    page_t pages[];
} c_queue_t;


/*============================================================================
 * Policy Implementation
 */


/* The circular queue of pages that are currently resident. */
static c_queue_t *c_queue;


/* Initialize the policy.  Return nonzero for success, 0 for failure. */
int policy_init(int max_resident) {
    fprintf(stderr, "Using CLOCK eviction policy.\n\n");
    
    /* use calloc() to set everything to zero */
    int size = sizeof(c_queue_t) + max_resident * sizeof(page_t);
    c_queue = malloc(size);

    if (c_queue) {
        /* zeros out the whole memory for future use */
        memset((void *) c_queue, 0, size);

        c_queue->max_resident = max_resident;
        /* initially clock_hand points to the first element */
        c_queue->clock_hand = 0;
    }
    
    /* Return nonzero if initialization succeeded. */
    return (c_queue != NULL);
}


/* Clean up the data used by the page replacement policy. */
void policy_cleanup(void) {
    free(c_queue);
    c_queue = NULL;
}


/* This function is called when the virtual memory system maps a page into the
 * virtual address space.  Record that the page is now resident.
 */
void policy_page_mapped(page_t page) {
    /* make sure that the current position in circular queue to put in new 
     * page is unoccupied.
     */
    assert(c_queue->pages[c_queue->clock_hand] == 0);
    c_queue->pages[c_queue->clock_hand] = page;

    /* moves clock_hand circularly */
    c_queue->clock_hand = (c_queue->clock_hand + 1) % c_queue->max_resident;
}


/* This function is called when the virtual memory system has a timer tick. */
void policy_timer_tick(void) {
    /* Do nothing! */
}


/* Choose a page to evict from the collection of mapped pages.  Then, record
 * that it is evicted.  
 */
page_t choose_and_evict_victim_page(void) {

    page_t victim;

    /* Figure out which page to evict. */
    while (1) {
        if (is_page_accessed(c_queue->pages[c_queue->clock_hand])) {
            /* if the current page is accessed, clear the bit and move on */
            clear_page_accessed(c_queue->pages[c_queue->clock_hand]);
            c_queue->clock_hand = 
                (c_queue->clock_hand + 1) % c_queue->max_resident;
        }
        else {
            /* otherwise it is the victim */
            break;
        }
    }
    victim = c_queue->pages[c_queue->clock_hand];

    /* zero out the victim page for future mapped page */
    c_queue->pages[c_queue->clock_hand] = 0;

#if VERBOSE
    fprintf(stderr, "Choosing victim page %u to evict.\n", victim);
#endif

    return victim;
}

