/******************************************************************************
    
        dmalloc hybrid arena allocator.

    Project repo at https://github.com/sizeof-dario/dmalloc.git.

******************************************************************************/

/*  "dmalloc_internals.h"

    Contains internal definitions for the allocator.                         */

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

/*  General purpose macros. */

#define UNUSED(param) (void)(param)
#define MIN(a, b) ((a) < (b) ? (a) : (b))

/*  *********************** */



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
    blockheader        *bhdr_first;
    void             *(*brkshifter)(intptr_t, struct arenaheader *);
} arenaheader;



void *arenasbrk(intptr_t delta, arenaheader *ahdr);

int meminit(void *backing_memory, size_t capacity);

void do_split(blockheader *bhdr, size_t bhdr_payload_size);

void *dmalloc_unlocked(size_t size, arenaheader *ahdr);

blockheader *do_coalesce_right(blockheader *bhdr);

void dfree_unlocked(void *p, arenaheader *ahdr);



#define ALIGN(val)  \
    (((val) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))

#define AL_ARENAHDR_SIZE ALIGN(sizeof(arenaheader))

#define AL_BLOCKHDR_SIZE ALIGN(sizeof(blockheader))

#define GET_ARENAHDR(ptr) (arenaheader *)ALIGN((uintptr_t)ptr)

#define GET_BLOCKHDR(ptr) (blockheader *)((uintptr_t)ptr - AL_BLOCKHDR_SIZE)

#define MIN_BLOCK_SIZE (AL_BLOCKHDR_SIZE + ALIGN(sizeof(char)))



#endif /* DMALLOC_INTERNALS_H */
