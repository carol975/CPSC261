#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "implicit.h"

/*
 * Determine whether or not a block is in use.
 */
static int block_is_in_use(char *block_start)
{
    return 1 & *((long *) block_start);
}

/*
 * Return the size of a block.
 */
static long get_block_size(char *block_start)
{
    return -8 & *((long *) block_start);
}

/*
 * Set the size of a block, and whether or not it is in use. Remember each block
 * has two copies of the header (one at each end).
 */
static void set_block_header(char *block_start, long block_size, int in_use)
{
    long header_value = block_size | in_use;
    *((long *) block_start) = header_value;
    *((long *) (block_start + (block_size - sizeof(long)) / sizeof(char))) = header_value;
}

/*
 * Create a heap that is "size" bytes large.
 */
heap *heap_create(long size, int search_alg)
{
    heap *h = malloc(sizeof(heap));

    h->size = size;
    h->start = malloc(size);
    h->search_alg = search_alg;
    
    h->next = h->start;
    set_block_header(h->start, size, 0);
    return h;
}

/*
 * Dispose of (free) the whole heap.
 */
void heap_dispose(heap *h)
{
    free(h->start);
    free(h);
}

/*
 * Find the start of the block, given a pointer to the payload.
 */
static char *get_block_start(char *payload)
{
    return payload - sizeof(long) / sizeof(char);
}

/*
 * Find the payload, given a pointer to the start of the block.
 */
static char *get_payload(char *block_start)
{
    return block_start + sizeof(long) / sizeof(char);
}

/*
 * Find the start of the next block.
 */
static char *get_next_block(char *block_start)
{
    return block_start + get_block_size(block_start) / sizeof(char);
}

/*
 * Find the start of the previous block.
 */
static char *get_previous_block(char *block_start)
{
    /* TO BE COMPLETED BY THE STUDENT. */
    return NULL;
}

/*
 * Coalesce two consecutive free blocks. Return a pointer to the beginning
 * of the coalesced block.
 */
static char *coalesce(char *first_block_start, char *second_block_start)
{
    /* TO BE COMPLETED BY THE STUDENT. */
    return NULL;
}

/*
 * Determine whether or not the given block is at the front of the heap.
 */
static int is_first_block(heap *h, char *block_start)
{
    return block_start == h->start;
}

/*
 * Determine whether or not the given block is at the end of the heap.
 */
static int is_last_block(heap *h, char *block_start)
{
    return block_start + get_block_size(block_start) == h->start + h->size;
}

/*
 * Print the structure of the heap to the screen.
 */
void heap_print(heap *h)
{
    /* TO BE COMPLETED BY THE STUDENT. */
}

/*
 * Determine the average size of a free block.
 */
long heap_find_avg_free_block_size(heap *h)
{
    /* TO BE COMPLETED BY THE STUDENT. */
    return 0;
}

/*
 * Free a block on the heap h. Beware of the case where the  heap uses
 * a next fit search strategy, and h->next is pointing to a block that
 * is to be coalesced.
 */
void heap_free(heap *h, char *payload)
{
    /* TO BE COMPLETED BY THE STUDENT. */
}

/*
 * Determine the size of the block we need to allocate given the size
 * the user requested. Don't forget we need space for the header  and
 * footer.
 */
static long get_size_to_allocate(long user_size)
{
    /* TO BE COMPLETED BY THE STUDENT. */
    return 16;
}

/*
 * Turn a free block into one the user can utilize. Split the block if
 * it's more than twice as large or MAX_UNUSED_BYTES bytes larger than
 * needed.
 */
static void *prepare_block_for_use(char *block_start, long real_size)
{
    /* TO BE COMPLETED BY THE STUDENT. */
    return NULL;
}

/*
 * Malloc a block on the heap h, using first fit. Return NULL if no block
 * large enough to satisfy the request exits.
 */
static void *malloc_first_fit(heap *h, long user_size)
{
    /* TO BE COMPLETED BY THE STUDENT. */
    return NULL;
}

/*
 * Malloc a block on the heap h, using best fit. Return NULL if no block
 * large enough to satisfy the request exits.
 */
static void *malloc_best_fit(heap *h, long user_size)
{
    /* TO BE COMPLETED BY THE STUDENT. */
    return NULL;
}

/*
 * Malloc a block on the heap h, using next fit. Return NULL if no block
 * large enough to satisfy the request exits.
 */
static void *malloc_next_fit(heap *h, long user_size)
{
    /* TO BE COMPLETED BY THE STUDENT. */
    return NULL;
}

/*
 * Our implementation of malloc.
 */
void *heap_malloc(heap *h, long size)
{
    switch (h->search_alg)
    {
        case HEAP_FIRSTFIT: return malloc_first_fit(h, size);
	case HEAP_NEXTFIT : return malloc_next_fit(h, size);
	case HEAP_BESTFIT : return malloc_best_fit(h, size);
    }
    return NULL;
}