/*============================================================================
 * Implementation of the AGING page replacement policy.
 *
 * We don't mind if paging policies use malloc() and free(), just because it
 * keeps things simpler.  In real life, the pager would use the kernel memory
 * allocator to manage data structures, and it would endeavor to allocate and
 * release as little memory as possible to avoid making the kernel slow and
 * prone to memory fragmentation issues.
 *
 * On periodic time interval, the AGING policy traverses all pages, and does
 * (a) shift each page's age value right by 1 bit, and the topmost bit is set
 * to the page's current accessed value.
 * (b) is a page's accessed value is set, clear it.
 * Then, when a page is to be evicted, the page with the lowest age value is
 * chosen.
 */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vmpolicy.h"
#include "virtualmem.h"

/* To extract or set the topmost bit of age value */
#define ACCESS_MASK 0x80

/*============================================================================
 * Page Data Structure
 *
 * A very simple structure to include an age value. Note that the age value
 * can be of various sizes, e.g., 8, 16, 32 bits. Here we choose 8 bits.
 */

typedef struct p_array_t {
    page_t page;
    uint8_t age;
} p_array_t;


/*============================================================================
 * "Loaded Pages" Data Structure
 *
 * This data structure records all pages that are currently loaded in the
 * virtual memory, so that we can choose a page to evict very easily.
 */

typedef struct loaded_pages_t {
    /* The maximum number of pages that can be resident in memory at once. */
    int max_resident;
    
    /* The number of pages that are currently loaded.  This can initially be
     * less than max_resident.
     */
    int num_loaded;
    
    /* This is the array of pages that are actually loaded.  Note that only the
     * first "num_loaded" entries are actually valid. And each element would
     * also contain an age value.
     */
    p_array_t pages[];
} loaded_pages_t;


/*============================================================================
 * Policy Implementation
 */


/* The list of pages that are currently resident. */
static loaded_pages_t *loaded;


/* Initialize the policy.  Return nonzero for success, 0 for failure. */
int policy_init(int max_resident) {
    fprintf(stderr, "Using AGING eviction policy.\n\n");
    
    int size = sizeof(loaded_pages_t) + max_resident * sizeof(p_array_t);
    loaded = malloc(size);
    if (loaded) {
        loaded->max_resident = max_resident;
        loaded->num_loaded = 0;
    }
    
    /* Return nonzero if initialization succeeded. */
    return (loaded != NULL);
}


/* Clean up the data used by the page replacement policy. */
void policy_cleanup(void) {
    free(loaded);
    loaded = NULL;
}


/* This function is called when the virtual memory system maps a page into the
 * virtual address space.  Record that the page is now resident.
 */
void policy_page_mapped(page_t page) {
    assert(loaded->num_loaded < loaded->max_resident);
    loaded->pages[loaded->num_loaded].page = page;

    /* When a page is paged in, the age value is set so the topmost bit is 1. */
    loaded->pages[loaded->num_loaded].age = ACCESS_MASK;
    loaded->num_loaded++;
}


/* This function is called when the virtual memory system has a timer tick. */
void policy_timer_tick(void) {
    /* Traverse the pages, and modify their age value. */
    for (int i = 0; i < loaded->num_loaded; i++) {
        assert(is_page_resident(loaded->pages[i].page));

        /* right shift */
        loaded->pages[i].age >>= 1;
        // printf("page %d have age %d\n", loaded->pages[i].page, loaded->pages[i].age);

        /* set the topmost bit according to accessed bit */
        if (is_page_accessed(loaded->pages[i].page)) {

            loaded->pages[i].age = loaded->pages[i].age | ACCESS_MASK;

            /* clear the accessed bit, and set the permission in order to
             * detect the next access.
             */
            clear_page_accessed(loaded->pages[i].page);
            set_page_permission(loaded->pages[i].page, PAGEPERM_NONE);
        }
    }
}


/* Choose a page to evict from the collection of mapped pages.  Then, record
 * that it is evicted.  
 */
page_t choose_and_evict_victim_page(void) {

    page_t victim;
    int i_victim;
    uint8_t min = 0xFF;

    /* Figure out which page to evict. Here, we pick the page with min age. */
    for (int i = 0; i < loaded->num_loaded; i++) {
        if (loaded->pages[i].age < min) {
            i_victim = i;
            min = loaded->pages[i].age;
        }
    }
    victim = loaded->pages[i_victim].page;

    /* Shrink the collection of loaded pages now, by moving the last page in the
     * collection into the spot that the victim just occupied.
     */
    loaded->num_loaded--;
    loaded->pages[i_victim] = loaded->pages[loaded->num_loaded];

#if VERBOSE
    fprintf(stderr, "Choosing victim page %u to evict.\n", victim);
#endif

    return victim;
}

