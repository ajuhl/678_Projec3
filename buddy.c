/**
 * Buddy Allocator
 *
 * For the list library usage, see http://www.mcs.anl.gov/~kazutomo/list/
 */

/**************************************************************************
 * Conditional Compilation Options
 **************************************************************************/
#define USE_DEBUG 1

/**************************************************************************
 * Included Files
 **************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>

#include "buddy.h"
#include "list.h"

/**************************************************************************
 * Public Definitions
 **************************************************************************/
#define MIN_ORDER 12
#define MAX_ORDER 20

#define PAGE_SIZE (1<<MIN_ORDER)
/* page index to address */
#define PAGE_TO_ADDR(page_idx) (void *)((page_idx*PAGE_SIZE) + g_memory)

/* address to page index */
#define ADDR_TO_PAGE(addr) ((unsigned long)((void *)addr - (void *)g_memory) / PAGE_SIZE)

/* find buddy address */
#define BUDDY_ADDR(addr, o) (void *)((((unsigned long)addr - (unsigned long)g_memory) ^ (1<<o)) \
									 + (unsigned long)g_memory)

#if USE_DEBUG == 1
#  define PDEBUG(fmt, ...) \
	fprintf(stderr, "%s(), %s:%d: " fmt,			\
		__func__, __FILE__, __LINE__, ##__VA_ARGS__)
#  define IFDEBUG(x) x
#else
#  define PDEBUG(fmt, ...)
#  define IFDEBUG(x)
#endif

/**************************************************************************
 * Public Types
 **************************************************************************/
 typedef struct {
 	struct list_head list;
 	int blockOrder;
 	char* blockAddress;
 		/* TODO: DECLARE NECESSARY MEMBER VARIABLES */

 } page_t;

/**************************************************************************
 * Global Variables
 **************************************************************************/
/* free lists*/
struct list_head free_area[ MAX_ORDER+1 ];

/* memory area */
char g_memory[1<<MAX_ORDER];

/* page structures */
 page_t g_pages[(1<<MAX_ORDER)/PAGE_SIZE];

/**************************************************************************
 * Public Function Prototypes
 **************************************************************************/

/**************************************************************************
 * Local Functions
 **************************************************************************/

/**
 * Initialize the buddy system
 */
void buddy_init()
{
	int i;
	int n_pages = (1<<MAX_ORDER) / PAGE_SIZE;
	for (i = 0; i < n_pages; i++) {
        g_pages[i].blockAddress = PAGE_TO_ADDR(i);
				g_pages[i].blockOrder = -1;
	}

	/* initialize freelist */
	for (i = MIN_ORDER; i <= MAX_ORDER; i++) {
		INIT_LIST_HEAD(&free_area[i]);
	}

	/* add the entire memory as a freeblock */
	list_add(&g_pages[0].list, &free_area[MAX_ORDER]);
}

/**
 * Allocate a memory block.
 *
 * On a memory request, the allocator returns the head of a free-list of the
 * matching size (i.e., smallest block that satisfies the request). If the
 * free-list of the matching block size is empty, then a larger block size will
 * be selected. The selected (large) block is then splitted into two smaller
 * blocks. Among the two blocks, left block will be used for allocation or be
 * further splitted while the right block will be added to the appropriate
 * free-list.
 *
 * @param size size in bytes
 * @return memory block address
 */
void * buddy_alloc(int size)
{
	/* TODO: IMPLEMENT THIS FUNCTION */
	//1. Ascertain the free-block order which can satisfy the requested size. The block order for size x is ceil ( log2 (x))

	//Example: 60k -> block-order = ceil ( log2 (60k)) = ceil ( log2 (k x 2^5 x 2^10)) = order-16
	int block_order = ceil(log2(size));
	if(block_order > MAX_ORDER){
		return NULL;
	}

	//2. Iterate over the free-lists; starting from the order calculated in the above step.
	for(int i = block_order; i <= MAX_ORDER; i++){
		//If the free-list at the required order is not-empty, just remove the first page from that list and return
		//it to caller to satisfy the request
		//3. If the free-list at the required order is empty, find the first non-empty free-list with order > required-order.
		// Lets say that such a list exists at order-k
	  if(!list_empty( &free_area[i])){
	    page_t* left = list_entry(free_area[i].next, page_t, list);
	    left->blockOrder = block_order;
	    list_del(&left->list);
			//4. Remove a page from the order-k list and repeatedly break the page and populate the respective free-lists until
			// the page of required-order is obtained. Return that page to caller
			//(It would be good to encase this functionality in a separate function e.g. split)
			for(int j = i; j != block_order; j--){
				page_t* right = &g_pages[ADDR_TO_PAGE(BUDDY_ADDR(left->blockAddress, (j-1)))];
		    right->blockOrder = (j-1);
	    	list_add(&right->list, &free_area[(j-1)]);
			}
	    return ((void*) left->blockAddress );
	  }

	}
//5. If a non-empty free-list is not found, this is an error
	return NULL;
}
/**
 * Free an allocated memory block.
 *
 * Whenever a block is freed, the allocator checks its buddy. If the buddy is
 * free as well, then the two buddies are combined to form a bigger block. This
 * process continues until one of the buddies is not free.
 *
 * @param addr memory block address to be freed
 */

void buddy_free(void * addr)
{
	//1. Calculate the address of the buddy
	page_t* freePage = &g_pages[ADDR_TO_PAGE(addr)];

	//2. If the buddy is free, merge the two blocks i.e. remove the buddy from its free-list,
	//update the order of the page-at-hand and add the page to the relevant free-list
	int i = freePage->blockOrder;
	bool isFree = true;
	while(i < MAX_ORDER && isFree){
	  isFree = false;
	  page_t* buddy = &g_pages[ADDR_TO_PAGE(BUDDY_ADDR(freePage->blockAddress, i))];
	  struct list_head* pos;
	  list_for_each(pos, &free_area[i]){
	    if(list_entry(pos, page_t, list) == buddy){
	        isFree = true;
	    }
	  }
	  if(isFree){
			list_del(&buddy->list);
	    if(freePage>buddy){
	    	freePage = buddy;
	    }
			i++;
	  }
	}
	freePage->blockOrder = i;
	list_add(&freePage->list, &free_area[i]);
}

/**
 * Print the buddy system status---order oriented
 *
 * print free pages in each order.
 */
void buddy_dump()
{
	int o;
	for (o = MIN_ORDER; o <= MAX_ORDER; o++) {
		struct list_head *pos;
		int cnt = 0;
		list_for_each(pos, &free_area[o]) {
			cnt++;
		}
		printf("%d:%dK ", cnt, (1<<o)/1024);
	}
	printf("\n");
}
