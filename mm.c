/*
 * Dynamic memory allocator using explicit lists
 *
 * This is adapted from the books implicit list implementation into an explicit list implementation
 * We create our explicit list by linking free blocks together in the 2 first words of a free block, as so:
 * |-----------------------|
 * |hdr |next|prev|data|ftr|
 * |-----------------------|
 * The header and footer contain the size and allocation state
 * The next pointer points to the next block in the linked list, or null if it's the root.
 * The prev pointer points to the previous block in the linked list, or null if it's the last block of the list.
 * The method follows the last in first out (LIFO) principle
 *
 * Another difference in this implementation is the attempt to uses succeeding blocks in realloc, to improve memory usages.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memlib.h"
#include "mm.h"

team_t team = {
    "albn",
    "Albert Rise Nielsen",
    "albn@itu.dk",
    "",
    ""};

// Size constants
#define WSIZE 4             // Word size in bytes. Also used as the header and footer size.
#define DSIZE 8             // Double word size
#define CHUNKSIZE (1 << 12) // Used as the size to extend the heap with. 4096 bytes.

// Get the max of 2 numbers
#define MAX(x, y) ((x) > (y) ? (x) : (y))

// Pack the size and allocated bit into a word. Used for headers and footers
// Packed together as:
// 16           2       0
// | size       | alloc |
#define PACK(size, alloc) ((size) | (alloc))

// Get a word address p. Used to read the header/footer
#define GET(p) (*(unsigned int *)(p))
// Write a word onto address p. Used to write the header/footer
#define PUT(p, val) (*(unsigned int *)(p) = (val))

// Read the size data of a block from the header/footer
#define GET_SIZE(p) (GET(p) & ~0x7) // Works by ignoring the first 3 bits, which are used by the allocator
// Read the allocation data of a block from the header/footer. Works by only getting the first bit.
#define GET_ALLOC(p) (GET(p) & 0x1)

// Compute the address of the header, from a pointer to the data location
#define HDRP(bp) ((char *)(bp)-WSIZE)
// Compute the address of the footer, from a pointer to the data location
#define FTRP(bp) ((char *)bp + GET_SIZE(HDRP(bp)) - DSIZE)

// Compute the location of the next block by the data location of a block
#define NEXT_BLKP(bp) ((char *)bp + GET_SIZE((char *)(bp)-WSIZE))
// Compute the location of the previous block by the data location of a block
#define PREV_BLKP(bp) ((char *)bp - GET_SIZE((char *)(bp)-DSIZE))

// Get the location of the pointer to the next block
#define NEXT_FBLKP(bp) ((char *)bp)
// Get the location of the pointer to the next block
#define PREV_FBLKP(bp) ((char *)bp+WSIZE)

// Get the pointer to the next free block
#define NEXT_FBLK(bp) ((void *)GET(NEXT_FBLKP(bp)))
// Get the pointer to the next free block
#define PREV_FBLK(bp) ((void *)GET(PREV_FBLKP(bp)))

static char *heap_listp = 0; // Pointer to the first block. Set in mm_init
static char *first_freep = 0; // Pointer to the first free block

// Prototypes, so we can call the methods before being defined
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
static void printblock(void *bp);
static void checkheap(int verbose, char name[]);
static void checkblock(void *bp);
static size_t get_alligned(size_t size);

static void set_next_fblkp(void *bp, void *next);
static void set_prev_fblkp(void *bp, void *next);
static void insert_in_empty_list(void *bp);
static void remove_from_empty_list(void *bp);

/*
 * mm_init - Initialize the memory manager
 */
int mm_init(void) {
  // Create the initial empty heap 
  if ((heap_listp = mem_sbrk(4 * WSIZE)) == (void *)-1)
    return -1;
  PUT(heap_listp, 0);                            // Alignment padding
  PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1)); // Prologue header
  PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1)); // Prologue footer
  PUT(heap_listp + (3 * WSIZE), PACK(0, 1));     // Epilogue header 
  heap_listp += (2 * WSIZE); // Placed on prologue footer

  // Extend the empty heap with a free block of CHUNKSIZE bytes 
  if ((first_freep = extend_heap(CHUNKSIZE / WSIZE)) == NULL)
    return -1;

  set_next_fblkp(first_freep, NULL);
  set_prev_fblkp(first_freep, NULL);

  return 0;
}

/*
 * mm_malloc - Allocate a block with at least size bytes of payload
 */
void *mm_malloc(size_t size) {
  size_t asize;      // Adjusted block size 
  size_t extendsize; // Amount to extend heap if no fit 
  void *bp;

  if (heap_listp == 0) {
    mm_init();
  }

  // Ignore spurious requests 
  if (size == 0)
    return NULL;

  asize = get_alligned(size);
  if ((bp = find_fit(asize)) == NULL) {

    extendsize = MAX(asize, CHUNKSIZE);
    if ((bp = extend_heap(extendsize / WSIZE)) == NULL)
      return NULL;
  }

  // No fit found. Get more memory and place the block 
  place(bp, asize);

  return bp;
}

/*
 * mm_free - Free a block
 */
void mm_free(void *bp) {
  // Cannot free the prologue block / alignment block which 0 points to
  if (bp == 0)
    return;

  // If the heap hasn't been initialized do so and return
  if (heap_listp == 0) {
    mm_init();
    return;
  }

  // Get the size of the current block
  size_t size = GET_SIZE(HDRP(bp));
  // Unallocate the block
  PUT(HDRP(bp), PACK(size, 0));
  PUT(FTRP(bp), PACK(size, 0));
  // Merge with sorrounding blocks
  coalesce(bp);
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp) {
  // Is the previous block allocated
  size_t prev_alloc = GET_ALLOC(FTRP(PREV_BLKP(bp)));
  // Is the next block allocated?
  size_t next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(bp)));
  // Get the size of the current block
  size_t size = GET_SIZE(HDRP(bp));

  // Sandwiched between 2 allocated blocks
  if (prev_alloc && next_alloc) {
    insert_in_empty_list(bp);
    return bp;
  }
  
  // Previous is allocated, but the next is free
  else if (prev_alloc && !next_alloc) {
    // Get the combined size of the current and next block
    size += GET_SIZE(HDRP(NEXT_BLKP(bp)));
    // Remove from empty list
    remove_from_empty_list(NEXT_BLKP(bp));
    // Overwrite the current block size
    PUT(HDRP(bp), PACK(size, 0));
    PUT(FTRP(bp), PACK(size, 0));
  }

  // Previous is free, but the next is allocated
  else if (!prev_alloc && next_alloc) {
    // Get the combined size of the current and next block
    size += GET_SIZE(HDRP(PREV_BLKP(bp)));
    // Remove from empty list
    remove_from_empty_list(PREV_BLKP(bp));
    // Overwrite the header of the previous block
    PUT(FTRP(bp), PACK(size, 0));
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    // Return the bp of previous blocks original position
    bp = PREV_BLKP(bp);
  } 

  // Both are free
  else {
    // Get the full size
    size += GET_SIZE(HDRP(PREV_BLKP(bp))) + GET_SIZE(FTRP(NEXT_BLKP(bp)));
    // Remove from empty list
    remove_from_empty_list(NEXT_BLKP(bp));
    remove_from_empty_list(PREV_BLKP(bp));
    // Overwrite the header of the previous block
    PUT(HDRP(PREV_BLKP(bp)), PACK(size, 0));
    // Overwrite the footer of the next block
    PUT(FTRP(NEXT_BLKP(bp)), PACK(size, 0));
    // Return the bp of previous blocks original position
    bp = PREV_BLKP(bp);
  }
  insert_in_empty_list(bp);

  // No change so return the current block
  return bp;
}

void *mm_realloc(void *ptr, size_t size) {
  void *newptr;
  size_t oldsize, asize, next_alloc, next_size;

  if (size == 0) {
    mm_free(ptr);
    return NULL;
  }

  if (ptr == NULL) {
    return mm_malloc(size);
  }

  oldsize = GET_SIZE(HDRP(ptr));
  asize = get_alligned(size);
  next_alloc = GET_ALLOC(HDRP(NEXT_BLKP(ptr)));
  next_size = GET_SIZE(HDRP(NEXT_BLKP(ptr)));

  if (asize == size) {
    return ptr;
  } else if (asize < oldsize) {
    // Smaller than current, cannot be shrunk
    // Smaller than current, can be shrunk and split
    return ptr;
  } else if (!next_alloc && ((next_size + oldsize) - 2 * DSIZE) >= asize) {
    // TODO: Why is 2*dsize necessary above? Check for possible cases where that impacts utility
    // Larger than current, next is last of heap, large enough, just not to split
    // Larger than current, next is last of heap, large enough, with split
    // Larger than current, next is free and large enough, just not to split
    // Larger than current, next is free and large enough, with split
    remove_from_empty_list(NEXT_BLKP(ptr));

    PUT(HDRP(ptr), PACK(next_size + oldsize, 1));
    PUT(FTRP(ptr), PACK(next_size + oldsize, 1));
    return ptr;
  } else {
    // TODO: Check that this actually uses the last block in case of expanding
    // Larger than current, next is not free, heap is small
    // Larger than current, next is not free, heap is large
    // Larger than current, next is free, but too small
    // Larger than current, next is last of heap, but too small
    // Larger than current, right next to end of heap
    if ((newptr = mm_malloc(asize)) == NULL)
      return NULL;

    place(newptr, asize);
    memcpy(newptr, ptr, oldsize);

    mm_free(ptr);

    return newptr;
  }

  return ptr;
}

static void set_next_fblkp(void *bp, void *next) {
  if (bp == NULL) return;

  PUT(NEXT_FBLKP(bp), next);
}

static void set_prev_fblkp(void *bp, void *prev) {
  if (bp == NULL) return;

  PUT(PREV_FBLKP(bp), prev);
}

/*
 * insert_in_empty_list - Insert free block in the list
 * We go by LIFO, so the inserted block shall have no previous node. Whilst we overwrite the previous of the last root block. Then we set our next to the previous root block and set the root to us.
 */
static void insert_in_empty_list(void *bp) {
  set_prev_fblkp(first_freep, bp);
  set_next_fblkp(bp, first_freep);
  set_prev_fblkp(bp, NULL);

  first_freep = bp;
}

static void remove_from_empty_list(void *bp) {
  void *prevp = PREV_FBLK(bp);
  void *nextp = NEXT_FBLK(bp);

  if (prevp == NULL) {
    set_prev_fblkp(nextp, NULL);
    first_freep = nextp;
  } else {
    set_next_fblkp(prevp, nextp);

    if (nextp != NULL)
      set_prev_fblkp(nextp, prevp);
  }

  set_next_fblkp(bp, 0);
  set_prev_fblkp(bp, 0);
}

/*
 * place - Place block of asize bytes at start of free block bp
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize)
{
  // Get the size of the block
  size_t csize = GET_SIZE(HDRP(bp));

  remove_from_empty_list(bp);
  // Split if there is space for another block, and its headers after our data
  if ((csize - asize) >= (2 * DSIZE)) {
    // Create the block for our data and allocate it
    PUT(HDRP(bp), PACK(asize, 1));
    PUT(FTRP(bp), PACK(asize, 1));
    // Create pointer for the block after
    bp = NEXT_BLKP(bp);
    // Create new free block
    PUT(HDRP(bp), PACK(csize - asize, 0));
    PUT(FTRP(bp), PACK(csize - asize, 0));

    coalesce(bp);
  } else {
    // Set the block as allocated
    PUT(HDRP(bp), PACK(csize, 1));
    PUT(FTRP(bp), PACK(csize, 1));
  }
}

/*
 * find_fit - Find a fit for a block with asize bytes
 */
static void *find_fit(size_t asize)
{
  // First-fit search 
  void *bp = first_freep;

  while (bp != NULL) {
    if (GET_SIZE(HDRP(bp)) >= asize)
      return bp;
    bp = NEXT_FBLK(bp);
  }

  return NULL;
}

/*
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words) {
  char *bp;
  size_t size;

  // Allocate an even number of words to maintain alignment 
  size = (words % 2) ? (words + 1) * WSIZE : words * WSIZE;
  if ((long)(bp = mem_sbrk(size)) == -1)
    return NULL;

  // Initialize free block header/footer and the epilogue header 
  // Overwrites old epilogue header, notice HDRP
  PUT(HDRP(bp), PACK(size, 0));         // Free block header 
  PUT(FTRP(bp), PACK(size, 0));         // Free block footer 
  PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); // New epilogue header 

  // Coalesce if the previous block was free 
  return coalesce(bp);
}

static void printblock(void *bp) {
  size_t hsize, halloc, fsize, falloc, nextfp, prevfp;

  checkheap(0, "");
  hsize = GET_SIZE(HDRP(bp));
  halloc = GET_ALLOC(HDRP(bp));
  fsize = GET_SIZE(FTRP(bp));
  falloc = GET_ALLOC(FTRP(bp));
  nextfp = GET(NEXT_FBLKP(bp));
  prevfp = GET(PREV_FBLKP(bp));

  if (hsize == 0) {
    printf("%p: EOL\n", bp);
    return;
  }

  printf("%p: header: [%ld:%c] footer: [%ld:%c].", bp, hsize,
         (halloc ? 'a' : 'f'), fsize, (falloc ? 'a' : 'f'));
  if (!halloc) printf(" list: [%p:%p]\n", nextfp, prevfp);
  else printf("\n");
}

/*
 * mm_checkheap - Check the heap for correctness
 */
void mm_checkheap(int verbose) { checkheap(verbose, ""); }

static void checkblock(void *bp) {
  // The pointer must be doubleword aligned
  if ((size_t)bp % DSIZE)
    printf("Error: %p is not doubleword aligned\n", bp);
  // Header and footer location must be correct and contain the same data
  if (GET(HDRP(bp)) != GET(FTRP(bp)))
    printf("Error: header does not match footer\n");
}

/*
 * checkheap - Minimal check of the heap for consistency
 */
void checkheap(int verbose, char name[]) {
  char *bp = heap_listp;

  if (verbose) {
    printf("Checking heap for %s\n", name);
    // Print the location of the heap
    printf("Heap (%p):\n", heap_listp);
  }

  // Prologue header must be 2 words, and not allocated.
  if ((GET_SIZE(HDRP(heap_listp)) != DSIZE) || !GET_ALLOC(HDRP(heap_listp)))
    printf("Bad prologue header\n");
  // Check the prologue header
  checkblock(heap_listp);

  // Init bp to the end of the prologue header
  // Loop if the size of the header is above 0
  // For each loop set bp to the next block in memory
  // Basically: Loop the entire heap and check every block
  // NOTE: This loop is bad. The epilogue header hasn't been checked yet and could be incorrect
  // NOTE: If something has it's size set to 0 the loop would also end, not actually checking the entire heap.
  // TODO: One could store the heap info in the prologue header to check the correctness of the prologue and epilogue headers
  for (bp = heap_listp; GET_SIZE(HDRP(bp)) > 0; bp = NEXT_BLKP(bp)) {
    if (verbose)
      printblock(bp);
    // Check every single block
    checkblock(bp);
  }

  if (verbose)
    printblock(bp);
  // Size of epilogue must be 0
  // Epilogue must not allocated
  // NOTE: See note at loop, this could cause issues
  if ((GET_SIZE(HDRP(bp)) != 0) || !(GET_ALLOC(HDRP(bp))))
    printf("Bad epilogue header\n");

  if (verbose)
    printf("first_free: %p\n", first_freep);
  if (verbose && first_freep != NULL) {
    printf("Free list root:\n"); 
    printblock(first_freep);
  }
}

static size_t get_alligned(size_t size) {
  // Adjust allocation size to be doubleword alligned
  if (size <= DSIZE)
    return 2 * DSIZE;
  else
    // Abuse integer division to adjust
    // mm_malloc 10
    // 8 * ((10 + 8 + (8 - 1)) / 8)
    // 8 * ((10 + 8 + 7) / 8)
    // 8 * (25 / 8)
    // 8 * 4
    // 32
    return DSIZE * ((size + (DSIZE) + (DSIZE - 1)) / DSIZE);
}
