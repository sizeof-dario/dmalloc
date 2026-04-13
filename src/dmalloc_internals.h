/*
*       dmalloc allocator.
*
*   Project repository at https://github.com/sizeof-dario/dmalloc.git.
*
*   You can read the documentation at https://sizeof-dario.github.io/dmalloc/.
*
*******************************************************************************/

/*      "dmalloc_internals.h"                                                 */

#ifndef DMALLOC_INTERNALS_H
#define DMALLOC_INTERNALS_H 1

#ifndef _DEFAULT_SOURCE
 /* Feature test macro required for sbrk(). */
 #define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <pthread.h>
#include <stdint.h>
#include <stdalign.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

/*  Headers. *******************************************************************/

typedef struct blockheader
{
    size_t              payload_size;
    int                 is_free;
    struct blockheader *bhdr_prev;
    struct blockheader *bhdr_next;
} blockheader;

typedef struct arenaheader
{
    pthread_mutex_t     lock;
    void               *arena_start;
    void               *arena_brk;
    void               *arena_end;
    void             *(*brkshifter)(intptr_t, struct arenaheader *);
    blockheader        *bhdr_first;
} arenaheader;

/*  ***************************************************************************/



/*  Allocator macros. *********************************************************/

#define ALIGN(x) (((x) + alignof(max_align_t)-1) & ~(alignof(max_align_t)-1))

#define AL_ARENAHDR_SIZE ALIGN(sizeof(arenaheader))
#define AL_BLOCKHDR_SIZE ALIGN(sizeof(blockheader))

#define MIN_BLOCK_SIZE (AL_BLOCKHDR_SIZE + ALIGN(1))

#define GET_ARENAHDR(p) (arenaheader *)ALIGN((uintptr_t)(p))
#define GET_BLOCKHDR(p) (blockheader *)((uintptr_t)(p) - AL_BLOCKHDR_SIZE)

/*  ***************************************************************************/



/*  Allocator internal functions. *********************************************/

/*  Shifts by `delta` bytes the break of the arena whose header is pointed to by
    `ahdr`. */
void *arenasbrk(intptr_t delta, arenaheader *ahdr);



/*  Initializes the memory region pointed to by `backing_memory` and of size
    `capacity`. If `backing_memory` is NULL, the heap is initialized. */
int backingmemoryinit(void *backing_memory, size_t capacity);



/*  Performs block spliting on the block whose header is pointed to by `bhdr`.
    `size` determines how much space must remain in the block that is split.
    No check is done about if block splitting can happen. */
void do_split(blockheader *bhdr, size_t size);



/*  Allocates `size` bytes of memory on the arena whose header is pointed to by
    `ahdr` or on the heap if `ahdr` is NULL. It's not thread-safe. */
void *dmalloc_unlocked(size_t size, arenaheader *ahdr);



/*  Performs block coalescing on the right on the block whose header is pointed
    to by `bhdr`. No check is done about if block coalescing can happen. */
blockheader *do_coalesce_right(blockheader *bhdr);



/*  Frees memory pointed to by `p` fro, the arena whose header is pointed to by
    `ahdr`. If `ahdr` is NULL, memory is freed from the heap. */
void dfree_unlocked(void *p, arenaheader *ahdr);

#endif /* DMALLOC_INTERNALS_H */
