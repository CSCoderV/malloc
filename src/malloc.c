#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)     ((b) + 1)
#define BLOCK_HEADER(ptr) ((struct _block *)(ptr) - 1)

static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
static int num_splits        = 0;
static int num_coalesces     = 0;
static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("splits:\t\t%d\n", num_splits );
  printf("coalesces:\t%d\n", num_coalesces );
  printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block 
{
   size_t  size;         /* Size of the allocated _block of memory in bytes     */
   struct _block *next;  /* Pointer to the next _block of allocated memory      */
   bool   free;          /* Is this _block free?                                */
   char   padding[3];    /* Padding: IENTRTMzMjAgU3jMDEED                       */
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */

/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes 
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \TODO Implement Next Fit
 * \TODO Implement Best Fit
 * \TODO Implement Worst Fit
 */

struct _block *findFreeBlock(struct _block **last, size_t size) 
{
   struct _block *curr = heapList;

#if defined FIT && FIT == 0
   /* First fit */
   //
   // While we haven't run off the end of the linked list and
   // while the current node we point to isn't free or isn't big enough
   // then continue to iterate over the list.  This loop ends either
   // with curr pointing to NULL, meaning we've run to the end of the list
   // without finding a node or it ends pointing to a free node that has enough
   // space for the request.
   // 
   while (curr && !(curr->free && curr->size >= size)) 
   {
      *last = curr;
      curr  = curr->next;
   }
#endif

// \TODO Put your Best Fit code in this #ifdef block
#if defined BEST && BEST == 0
   /** \TODO Implement best fit here */

   struct _block *best_ptr= NULL;
   while (curr){ 
      if (curr->free && curr->size>=size){
         if (!best_ptr ||curr->size <best_ptr->size ){
            best_ptr=curr;
         }
         *last=curr;
         curr=curr->next;
      }
   }
   curr=best_ptr;
      if (curr)
         num_reuses++;
#endif

// \TODO Put your Worst Fit code in this #ifdef block
#if defined WORST && WORST == 0
   /** \TODO Implement worst fit here */
   struct _block*worst_ptr= NULL;
   while (curr){    
      if (curr->free && curr->size>=size){
         if (!worst_ptr || curr->size>worst_ptr->size)
            worst_ptr= curr;  
      }
      *last=curr;
      curr=curr->next;
   }
   curr=worst_ptr;
   if (curr)
      num_reuses++;
#endif

// \TODO Put your Next Fit code in this #ifdef block
#if defined NEXT && NEXT == 0
   /** \TODO Implement next fit here */
   static struct _block *last_use=NULL;
   if (!last_use)
      last_use=heapList; 
   struct _block *start= last_use;
   int first_pass=1;
   while (last_use && (first_pass || last_use!=start)){
      first_pass=0;
      if (last_use->free && last_use->size>=size){
         curr=last_use;
         num_reuses++;
         break;
      }
      *last=last_use;
      if (last_use->next)
         last_use=last_use->next;
      else
         last_use=heapList;
   }
   if (curr==NULL)
      *last = last_use;
   if (curr)
      last_use= curr->next;
   else
      last_use=  *last;
#endif

return curr;
}

/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically 
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size) 
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1) 
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL) 
   {
      heapList = curr;
   }

   /* Attach new _block to previous _block */
   if (last) 
   {
      last->next = curr;
   }

   /* Update _block metadata:
      Set the size of the new block and initialize the new block to "free".
      Set its next pointer to NULL since it's now the tail of the linked list.
   */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;
   return curr;
}

/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the 
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process 
 * or NULL if failed
 */
void *malloc(size_t size) 
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0) 
   {
      return NULL;
   }

   /* Look for free _block.  If a free block isn't found then we need to grow our heap. */

   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);

   /* TODO: If the block found by findFreeBlock is larger than we need then:
            If the leftover space in the new block is greater than the sizeof(_block)+4 then
            split the block.
            If the leftover space in the new block is less than the sizeof(_block)+4 then
            don't split the block.
   */

   /* Could not find free _block, so grow heap */
   if (next == NULL) 
   {
      next = growHeap(last, size);
      num_grows++;
      num_blocks++;
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL) 
   {
      return NULL;
   }
   if (next->size>=size + sizeof(struct _block)+4){
      struct _block *old_next= next->next;
      size_t old_size=next->size;
      struct _block *new_block = (struct _block *)((char *)BLOCK_DATA(next)+size);
      new_block->size=old_size-size-sizeof(struct _block);
      new_block->next=old_next;
      new_block->free=true;
      next->size=size;
      next->next=new_block;
      num_splits++;
      num_blocks++;
   }
   /* Mark _block as in use */
   next->free = false;
   num_mallocs++;
   num_requested+=size;
   int heap_size=(char*) next + sizeof(struct _block)+ next->size- (char*) heapList;
   if (heap_size> max_heap)
      max_heap=heap_size;

   /* Return data address associated with _block to the user */
   return BLOCK_DATA(next);
}

/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr) 
{
   if (ptr == NULL) 
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;
   num_frees++; 
   /* TODO: Coalesce free _blocks.  If the next block or previous block 
            are free then combine them with this block being freed.
   */
   if (curr->next && curr->next->free) 
   {
      curr->size += sizeof(struct _block) + curr->next->size;
      curr->next = curr->next->next;
      num_coalesces++;
      num_blocks--;
   }
   struct _block *prev = NULL;
   struct _block *iter = heapList;
   while (iter && iter !=curr){
      if (iter->free)
         prev = iter;
      iter = iter->next;
   }
   if (prev && prev->free){
      prev->size+= sizeof(struct _block)+curr->size;
      prev->next= curr->next;
      num_coalesces++;
      num_blocks--;
   }
   
}

void *calloc( size_t nmemb, size_t size )
{
   // \TODO Implement calloc
   size_t total = nmemb * size;
   void *ptr1 = malloc(total);
   if (ptr1) 
   {
      memset(ptr1, 0, total);
   }
   return ptr1;
}

void *realloc( void *ptr1, size_t size )
{
   // \TODO Implement realloc
   if (!ptr1)
       return malloc(size);
   if (size == 0) {
       free(ptr1);
       return NULL;
   } 

   struct _block*current= BLOCK_HEADER(ptr1);
   if (current->size >= size)
       return ptr1;
   void *ptr2 = malloc(size);
   if (ptr2){
      memcpy(ptr2, ptr1, current->size);
      free(ptr1);
      return ptr2;
   }
   return NULL;
   
}



/* vim: IENTRTMzMjAgU3ByaW5nIDIwM002= ----------------------------------------*/
/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
