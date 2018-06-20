/*! \file
 * Implementation of a simple memory allocator.  The allocator manages a small
 * pool of memory, provides memory chunks on request, and reintegrates freed
 * memory back into the pool.
 *
 * Adapted from Andre DeHon's CS24 2004, 2006 material.
 * Copyright (C) California Institute of Technology, 2004-2010.
 * All rights reserved.
 */

#include <stdio.h>
#include <stdlib.h>
#include <limits.h>

#include "myalloc.h"


/*!
 * These variables are used to specify the size and address of the memory pool
 * that the simple allocator works against.  The memory pool is allocated within
 * init_myalloc(), and then myalloc() and free() work against this pool of
 * memory that mem points to.
 */
int MEMORY_SIZE;
unsigned char *mem;


/* EXPLICIT FREE LIST & BEST FIT ALLOCATION
 * This allocator implements an explicit free list, where all the free blocks
 * are chained together to form a linked list. Each time we are going to find
 * a block for a request, we simply traverse the linked list and choose the 
 * smallest free block which satisfies the request - the best fit strategy.
 * The list is formed by headers of the blocks, and the prev / next pointers
 * are stored in the headers as well.
 */
/* MEMORY BLOCK REPRESENTATION
 * This allocator represents memory blocks with header and footer, where the
 * header contains a 32-bit int value for size, with negative value indicating
 * allocated, and positive value indicating free (footer has exactly the same 
 * structure, except that it is located at the end of the memory block while 
 * the header is located at the start of the memory block). In addition, both
 * header and footer contains two pointers: the prev pointer points to the
 * previous header in the explicit free list, and the next pointer points to
 * the next element. If the block is currently not in the free list, both
 * pointers will be NULL.
 */
/* define a header type to reference headers of memory blocks. */
typedef struct header {
    /* Negative size means the block is allocated, */
    /* positive size means the block is available. */
    int size;

    /* points to the previous element in the free list. */
    struct header *prev;

    /* points to the next element in the free list. */
    struct header *next;
} header;

/* 
 * declare two global variables: head and tail of the free list. They are both
 * pointer of headers, with prev = NULL for head and next = NULL for tail.
 */
header *list_head;
header *list_tail;


/* SANITY-CHECK FUNCTION
 * a verification function which traverses the heap and computes the sum of 
 * space and checks if the sum matches the memory pool size.
 */
void sanity_check();

void sanity_check() {
    
    unsigned char *test_ptr = mem;

    /* 
     * in the loop, examine whether the size in the header matches with the
     * size in the footer, if not, break and print error.
     */
    while (test_ptr < mem + MEMORY_SIZE) {
        header *h = (header *) test_ptr;
        test_ptr += sizeof(header) + abs(h->size);
        header *f = (header *) test_ptr;
        if (f->size != h->size) {
            printf("footer size does not match header size at %s\n", test_ptr);
            break;        
        }
        test_ptr += sizeof(header);    
    }

    /* after the loop, check if the total size matches with traverse length */
    if (test_ptr != mem + MEMORY_SIZE) {
        printf("the total size does not match\n");    
    }
    // printf("Sanity Check: OK!\n");
}


/*
 * This function moves out the header element from the linked list, and
 * maintains the abstraction of list_head and list_tail
 */
void move_out(header *h);

void move_out(header *h) {
    if (h->prev == NULL && h-> next == NULL) {
        /* only one block exists */
        list_head = NULL;
        list_tail = NULL;
    }
    else if (h->prev == NULL) {
        /* the first block is the best */
        list_head = h->next;
        list_head->prev = NULL;        
    }
    else if (h->next == NULL) {
        /* the last block is the best */
        list_tail = h->prev;
        list_tail->next = NULL;
    }
    else {
        h->prev->next = h->next;
        h->next->prev = h->prev;        
    }
}

/*
 * This function puts a header element into the linked list, and maintains
 * the abstraction of list_head and list_tail
 */
void put_in(header *h, header *f);

void put_in(header *h, header *f) {
    if (list_tail == NULL) {
        /* no element in the list */
        list_tail = h;
        list_head = h;
        h->prev = NULL;
        h->next = NULL;
        f->prev = NULL;
        f->next = NULL;            
    }
    else {
        list_tail->next = h;
        h->prev = list_tail;
        h->next = NULL;
        list_tail = h;
        f->prev = h->prev;
        f->next = h->next;            
    }
}


/*!
 * This function initializes both the allocator state, and the memory pool.  It
 * must be called before myalloc() or myfree() will work at all.
 *
 * Note that we allocate the entire memory pool using malloc().  This is so we
 * can create different memory-pool sizes for testing.  Obviously, in a real
 * allocator, this memory pool would either be a fixed memory region, or the
 * allocator would request a memory region from the operating system (see the
 * C standard function sbrk(), for example).
 */
void init_myalloc() {

    /*
     * Allocate the entire memory pool, from which our simple allocator will
     * serve allocation requests.
     */
    mem = (unsigned char *) malloc(MEMORY_SIZE);
    if (mem == 0) {
        fprintf(stderr,
                "init_myalloc: could not get %d bytes from the system\n",
		MEMORY_SIZE);
        abort();
    }

    /* Set the header and the footer for the whole memory block. */
    header *h = (header *) mem;
    h->size = MEMORY_SIZE - (sizeof(header)) * 2;
    header *f = (header *) (mem + MEMORY_SIZE - sizeof(header));
    f->size = h->size;

    /* 
     * Initialize the explicit free list with a single element:
     * the whole block. So the header is both head and tail of
     * the free list.
     */
    h->prev = NULL;
    h->next = NULL;
    f->prev = NULL;
    f->next = NULL;
    list_head = h;
    list_tail = h;
}


/*!
 * Attempt to allocate a chunk of memory of "size" bytes.  Return 0 if
 * allocation fails.
 */
unsigned char *myalloc(int size) {
 
    /* FINDING SUITABLE FREE BLOCKS
     * inside myalloc function, we traverse the sequence of blocks in the free
     * list, and choose the smallest block to satisfy the request.
     * i.e., currently we are using best-fit strategy.
     */
    header *traverse = list_head;
    int flag = 0;
    int best_size = INT_MAX;
    header *best_block = NULL;

    while (traverse != NULL) {
        int temp_size = traverse->size;        
        
        if (temp_size >= size) {
            /* find one */
            flag = 1;
            /* update the minimum size */
            if (temp_size < best_size) {
                best_size = temp_size;
                best_block = traverse;            
            }
        }

        traverse = traverse->next;
    }

    /* find one suitable memory block */
    if (flag) {

        /*
         * Now we find one block, first of all we drag this block out of the
         * free list. Here, due to the advantage of double linked list, we
         * simply let the previous element point to the next element.
         */
        move_out(best_block);

        /* 
         * If the free block size is much larger than the allocation size, 
         * split it into 2 blocks. 
         * (Version 2: split the block if the block is more than size + 100)
         */
        header *h_1 = best_block;
        int old_size = h_1->size;

        if (old_size > size + 100) {
            /* 
             * In this case, the entire free block is splitted into 2:
             * Block 1 (for allocation) and Block 2 (the remainder).
             */

            /* change the size and pointer of block 1 in header and footer */
            h_1->size = -size;
            h_1->prev = NULL;
            h_1->next = NULL;

            header *f_1 = (header *) ((char *) h_1 + sizeof(header) + size);
            f_1->size = -size;
            f_1->prev = NULL;
            f_1->next = NULL;
            
            /* change or build the header and the footer of the block 2 */
            header *h_2 = f_1 + 1;
            h_2->size = old_size - size - 2 * sizeof(header);
            header *f_2 = (header *)((char *) h_1 + sizeof(header) + old_size);
            f_2->size = h_2->size;

            /* move this memory block pointer and the free pointer correctly */
            unsigned char *resultptr = (unsigned char *) h_1 + sizeof(header);

            /* put the generated free block into the free list */
            put_in(h_2, f_2);
            
            sanity_check();
            return resultptr;
        }
        else {
            /* In this case, we don't split blocks, simply do a minus sign */        
            h_1->size = -old_size;
            header *f_1 = (header *)((char *) h_1 + sizeof(header) + old_size);
            f_1->size = -old_size;

            /* clear the pointers in header and footer */
            h_1->prev = NULL;
            h_1->next = NULL;
            f_1->prev = NULL;
            f_1->next = NULL;

            /* move this memory block pointer and the free pointer correctly */
            unsigned char *resultptr = (unsigned char *) h_1 + sizeof(header);

            sanity_check();
            return resultptr;
        }
    }
    /* we cannot find one, so we give out desperate message */
    else {
        fprintf(stderr, "myalloc: cannot service request of size %d\n", size);
        return (unsigned char *) 0;
    }
}


/*!
 * Free a previously allocated pointer.  oldptr should be an address returned by
 * myalloc().
 */
void myfree(unsigned char *oldptr) {
    
    /*! 
     * The deallocation strategy in this function is in constant-time:
     * each time we free a block, we simply look at its adajecent block
     * through their headers and footers, and do coalesce according to
     * the size inside the pointer. This is the advantage of header-and-
     * footer structure.
     * Since headers and footers also contain pointers to the free list,
     * we can directly move out / in element according to the pointer,
     * this operation is also in constant time thanks to the double linked
     * list.
     */

    /* DEALLOCATION V1: simply free the block and mark as free. */
    header *h = (header *) (oldptr - sizeof(header));

    /* check if the block is really occupied */
    if (h->size > 0) {
        printf("Hey, this block is free, no need to free it.\n");
        return;
    }    
    h->size = -h->size;

    header *f = (header *) (oldptr + h->size);
    f->size = -f->size;

    sanity_check();

    /* DEALLOCATION V2: forward coalesce, check if the next block is free */
    if ((unsigned char *) (f + 1) < mem + MEMORY_SIZE) {
        /* not go beyond the memory */
        header *h_next = f + 1;
        if (h_next->size > 0) {
            /* 
             * find an adjacent free memory block, change the size of the
             * header of the first block and the footer of the second block.
             */
            int temp_size = h->size;
            h->size += h_next->size + 2 * sizeof(header);
            header *f_next = (header *) ((char *)h_next + h_next->size 
                             + sizeof(header));
            f_next->size += temp_size + 2 * sizeof(header);

            /* drag the merged block out from the linked list */
            move_out(h_next);

            /* change pointer name to do backward coalesce */
            f = f_next;
            sanity_check();
        }
    }
    
    /* DEALLOCATION V3: backward coalesce, check if previous block is free */
    if ((unsigned char *) h > mem) {
        /* not go beyond the memory */
        header *f_prev = h - 1;
        if (f_prev->size > 0) {
            /* 
             * find an adjacent (previous) free memory block, change the size
             * of the header of the previous block and the footer of the
             * current block.
             */
            int temp_size = h->size;
            f->size += f_prev->size + 2 * sizeof(header);
            header *h_prev = (header *) ((char *)h - f_prev->size 
                             - 2 * sizeof(header));
            h_prev->size += temp_size + 2 * sizeof(header);

            /* drag the merged block out from the linked list */
            move_out(h_prev);

            /* change pointer name - they are one block now */
            h = h_prev;
            sanity_check();
        }    
    }

    /* put the coalesced block into the linked list */
    put_in(h, f);
}

/*!
 * Clean up the allocator state.
 * All this really has to do is free the user memory pool. This function mostly
 * ensures that the test program doesn't leak memory, so it's easy to check
 * if the allocator does.
 */
void close_myalloc() {
    free(mem);
}
