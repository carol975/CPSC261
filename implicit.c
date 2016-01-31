#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#include "implicit.h"

static long h_size;



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
	printf("\nthe header value is: %lu\n\n");
    *((long *) block_start) = block_size | in_use;
	printf("header value is : %lu\n",   *((long *) block_start) );
	//reaches header 2
    *((long *) (block_start + (block_size - sizeof(long)) / sizeof(char))) = block_size | in_use;
	printf("footer value is  : %lu\n",     *((long *) (block_start + (block_size - sizeof(long)) / sizeof(char))) );
}

/*
 * Create a heap that is "size" bytes large.
 */
heap *heap_create(long size, int search_alg)
{
    heap *h = malloc(sizeof(heap));

    h->size = size;
	h_size =  size;
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
  //if(is_last_block()){return NULL;}
  return block_start + get_block_size(block_start) / sizeof(char); // sizeof(char) because char is 1 byte long
}

/*
 * Find the start of the previous block.
 */
static char *get_previous_block(char *block_start)
{
  char *current_position = block_start;
  
  current_position = current_position - (sizeof(long));///sizeof(char));
  long prev_size =  *((long *) current_position);
  printf("prev size is %lu\n",prev_size);
  // move to beginning of previous block's footer
  //long prev_size = get_block_size(current_position); // should read previous block's size from its footer (?)
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
	printf("The new block size is  %lu\n",new_block_size);
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
  int  num_of_free_blocks = 0;
  char *current_position = h->start;

  while( 1 ) {

    if( !block_is_in_use(current_position) ) {
      sum += get_block_size(current_position);
      ++num_of_free_blocks;
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
		this_start = coalesce(this_start, next_start);
		this_size += get_block_size(next_start);
	}
	if(block_is_in_use(prev_start)){
		this_start = coalesce(prev_start, this_start);
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
  long most_wasted_space = 2147483647;

  while( current_position != NULL) {  
    
    if( !block_is_in_use(current_position) && get_block_size(current_position) >= (user_size+2*(sizeof(long)/sizeof(char))) ) {
      // compute how much wasted space:
      long wasted_space = get_block_size( current_position ) - (user_size + 2*(sizeof(long)/sizeof(char)));
      if( wasted_space < most_wasted_space ) { // if current position has less wasted space, then update
	  best_fit_position = current_position;
	  most_wasted_space = wasted_space;
      }
	  if( is_last_block( h , current_position ) ) { // exit while loop after reaching last block
      break;
		}  
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

void test(heap *h){
	printf("\nThis is the test for heap_find_avg_free_block_size\n");
    printf("\nAverage Block Size: %lu\n", heap_find_avg_free_block_size(h));
	printf("\n===============================\n");
	
	
	long first_bloc_size = get_block_size(h->start);
	printf("The first block size is %lu\n",first_bloc_size);
	char * allocated_bloc_start;
	//Test for get_size_to_allocate
	printf("\nThis is the test for get_size_to_allocate\n");
	long user_size = 64;
	long mem_size = get_size_to_allocate(user_size);
	printf("the size of two hearders is: %lu\n",2 * (sizeof(long)/sizeof(char)));
	printf("the size of user requrested memory is: %lu",user_size);
	printf("\nmemory size for %lu required is: %lu\n",user_size,mem_size);
	printf("\n===============================\n");

	
	//Test for malloc_first_fit
	printf("\nThis is the test for malloc_first_fit\n");
	
	printf("Allocating for a size of %lu\n",user_size);
	printf("\nThis is the heap before first fit malloc\n");
	printf("\n");
	heap_print(h);
	allocated_bloc_start = malloc_first_fit(h, user_size);
	printf("\nThis is the heap after first fit malloc\n");
	heap_print(h);
	
	printf("\n Allocating for a size of 9999999\n");
	allocated_bloc_start = malloc_first_fit(h, 9999999);
	printf("\nThis is the heap after first fit malloc\n");
	heap_print(h);
	printf("\n===============================\n");
	
	
	
	
	//Test for malloc_next_fit
	printf("\nThis is the test for malloc_next_fit\n");
	user_size = 56;
	printf("Allocating for a size of %lu\n",user_size);
	allocated_bloc_start = malloc_next_fit(h, user_size);
	printf("\nThis is the heap after next fit malloc\n");
	heap_print(h);
	printf("\n===============================\n");
	
	//Test for prepare_block_for_use
	printf("\nThis is the test for  ");
	//char *payload = (char*)88;
   //char *start1 = get_block_start(payload);
     //printf("start1 is: %lu\n",sizeof(start1));
	 //Test for malloc_best_Fit
	printf("\nThis is the test for malloc_best_fit\n");
	user_size = 40;
	printf("Allocating for a size of %lu\n",user_size);
	allocated_bloc_start = malloc_best_fit(h, user_size);
	printf("\nThis is the heap after best fit malloc\n");
	heap_print(h);
	printf("\n===============================\n");
	
		
	//Test for get_previous_block
	printf("\nThis is the test for get_previous_block\n");
	char *next = get_next_block(h->start);
	long next_block_size = get_block_size(next);
	printf("the size of the 2nd block is: %lu\n",next_block_size);
	
	
	char *previous = get_previous_block(next);
	long previous_block_size = get_block_size(previous);
	printf("the size of the previous lock of the 2nd block is: %lu\n",previous_block_size);
	printf("\n===============================\n");
	
	//Test for coalesce
	printf("\nThis is the test for coalesce\n");
	
	printf("This is the heap before coalescing first and second blocks\n");
	heap_print(h);
	char *first_bloc = h->start;
	char *first_payload = get_payload(first_bloc);
	
	char *second_bloc = get_next_block(first_bloc);
	char *second_payload = get_payload(second_bloc);
	
	//heap_free(h,first_payload);
	//heap_free(h,second_payload);
	
	char *coal_start = coalesce(first_bloc,second_bloc);
	
	printf("This is the heap after calescing first and second blocks\n");
	h->start = coal_start;
	heap_print(h);
	printf("\n===============================\n");
}
