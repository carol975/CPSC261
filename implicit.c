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
	
	//reaches header 2
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
    set_block_header(h->start, size, 0); // create header and footer for entire heap
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
 * Find the start of the block, given a pointer to the payload of a particular block.
 */
static char *get_block_start(char *payload)
{
    return payload - sizeof(long) / sizeof(char);
}

/*
 * Find the payload, given a pointer to the start of the required block.
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
  return block_start + get_block_size(block_start) / sizeof(char); // sizeof(char) because char is 1 byte long
}

/*
 * Find the start of the previous block.
 */
static char *get_previous_block(char *block_start)
{
  char *current_position = block_start;
  
  current_position = current_position - sizeof(long)/sizeof(char); // move to beginning of previous block's footer
  long prev_size = get_block_size(current_position); // should read previous block's size from its footer (?)
  current_position = current_position - prev_size + sizeof(long)/sizeof(char); // move to beginning of previous block

  return current_position;
}

/*
 * Coalesce two consecutive free blocks. Return a pointer to the beginning
 * of the coalesced block.
 */
static char *coalesce(char *first_block_start, char *second_block_start)
{
    long new_block_size = get_block_size(first_block_start) + get_block_size(second_block_start);
	set_block_header(first_block_start,new_block_size, 0 );
	
	//only freeing the pointer, not the memeory its pointing to
	free(second_block_start);
    return first_block_start;
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
  long size;
  int in_use;

  // step through each block and get address, size, and in use / not in use
  char *current_position = h->start;
  
  while( 1 ) {
    size = get_block_size(current_position);
    in_use = block_is_in_use(current_position);

    printf("\nBlock at address %x\n\tSize: %d\n\tIn use: ",current_position,size);
    if( in_use ) {
      printf("Yes");
    } else {
      printf("No");
    }

    if( is_last_block(h , current_position) ) {
      break;
    }
    current_position = get_next_block(current_position);
  }

  printf("\n\n\n");
}

/*
 * Determine the average size of a free block.
 */
long heap_find_avg_free_block_size(heap *h)
{
  // find the sum of sizes and the number of free blocks:
  long sum = 0;
  int num_of_free_blocks = 0;
  char *current_position = h->start;

  while( 1 ) {

    if( !block_is_in_use(current_position) ) {
      sum += get_block_size(current_position);
      num_of_free_blocks++;
    }

    if( is_last_block(h , current_position) ) {
      break;
    }
    current_position = get_next_block(current_position);  
  }

  if( num_of_free_blocks == 0 ) {
    return 0;
  } 

  return (sum / num_of_free_blocks);
}

/*
 * Free a block on the heap h. Beware of the case where the  heap uses
 * a next fit search strategy, and h->next is pointing to a block that
 * is to be coalesced. (mean "freed" (?))
 */
void heap_free(heap *h, char *payload)
{
	
	char * this_start = get_block_start(payload);
	char * prev_start  = get_previous_block(this_start);
	char * next_start = get_previous_block(this_start);
	
	int this_size;
	int prev_size;
	int next_size;
	
	if(block_is_in_use(next_start)){
		this_start = collesce(this_start, next_start);
		this_size += get_block_size(next_start);
	}
	if(block_is_in_use(prev_start)){
		this_start = collesce(prev_start, this_start);
		this_size += get_block_size(prev_start); 
	}
	set_block_header(this_start,this_size,0);
	h->start = this_start;
	free (prev_start);
	free (next_start);
	
  
}

/*
 * Determine the size of the block we need to allocate given the size
 * the user requested. Don't forget we need space for the header  and
 * footer.
 */
static long get_size_to_allocate(long user_size)
{
  return user_size + 2 * (sizeof(long)/sizeof(char));
}

/*
 * Turn a free block into one the user can utilize. Split the block if
 * it's more than twice as large or MAX_UNUSED_BYTES bytes larger than
 * needed.
 */
static char *prepare_block_for_use(char *block_start, long real_size)
{
  long old_block_size = get_block_size(block_start);

  if( old_block_size > 2*real_size || old_block_size > real_size+MAX_UNUSED_BYTES ) {
    // then split the block: 
    // rewrite header/footer for first half...
    set_block_header( block_start , real_size , 1 ); // "1" b/c assume allocated (?)

    // create header/footer for second half...
    set_block_header( get_next_block(block_start) , old_block_size - real_size , 0 ); // "0" b/c NOT allocated!
  } else {
    // prepare block without splitting:
    set_block_header( block_start , old_block_size , 1);
  }

  return block_start + sizeof(long) / sizeof(char); // return a pointer to the PAYLOAD 
}

/*
 * Malloc a block on the heap h, using first fit. Return NULL if no block
 * large enough to satisfy the request exits.
 */
static char *malloc_first_fit(heap *h, long user_size)
{
  char *current_position = h->start;
  while( block_is_in_use(current_position) || get_block_size(current_position) < user_size+2*(sizeof(long)/sizeof(char)) ) {
    if( is_last_block( h , current_position ) ) { // then no block of correct size exists
      return NULL;
    }
    current_position = get_next_block(current_position); // move to next block in memory allocator
  }
  return prepare_block_for_use( current_position , user_size ); // will split properly once suitable block found - points to payload
}

/*
 * Malloc a block on the heap h, using best fit. Return NULL if no block
 * large enough to satisfy the request exits.
 */
static char *malloc_best_fit(heap *h, long user_size)
{
  char *current_position = h->start;
  char *best_fit_position = NULL;
  long least_wasted_space = 2147483647;

  while( 1 ) {    
    if( !block_is_in_use(current_position) && get_block_size(current_position) >= user_size+2*(sizeof(long)/sizeof(char)) ) {
      // compute how much wasted space:
      long wasted_space = get_block_size( current_position ) - user_size - 2*(sizeof(long)/sizeof(char));
      if( wasted_space < least_wasted_space ) { // if current position has less wasted space, then update
	best_fit_position = current_position;
	least_wasted_space = wasted_space;
      }
    }
    if( is_last_block( h , current_position ) ) { // exit while loop after reaching last block
      break;
    }
    current_position = get_next_block(current_position);
  }

  if( best_fit_position == NULL ) {
    return NULL;
  }

  return prepare_block_for_use( best_fit_position , user_size );
}

/*
 * Malloc a block on the heap h, using next fit. Return NULL if no block
 * large enough to satisfy the request exits.
 */
static char *malloc_next_fit(heap *h, long user_size)
{
  char *current_position = h->next;
  while( block_is_in_use(current_position) || get_block_size(current_position) < user_size+2*(sizeof(long)/sizeof(char)) ) {
    if( is_last_block( h , current_position ) ) { // then no block of correct size exists
      return NULL;
    }
    current_position = get_next_block(current_position); // move to next block in memory allocator
  }
  h->next = current_position; // update the next pointer for following calls to malloc_next_fit
  return prepare_block_for_use( current_position , user_size ); // will split properly once suitable block found - points to payload 
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
