/*
    dmalloc memory allocator

    Check the project repo at https://github.com/sizeof-dario/dmalloc.git for
    more information about it.

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
#include <stdint.h>
#include <stdalign.h>
#include <stddef.h>
#include <unistd.h>

#define UNUSED(param) (void)(param)



typedef struct blockheader
{
    size_t              payload_size;
    int                 is_free;
    struct blockheader *bhdr_prev;
    struct blockheader *bhdr_next;
} blockheader;



typedef struct arenaheader
{
    void               *arena_start;
    void               *arena_brk;
    void               *arena_end;
    blockheader        *bhdr_first;
    void             *(*brkshifter)(intptr_t, struct arenaheader*);
} arenaheader;



void *arenasbrk(intptr_t increment, arenaheader *self);

int heapinit();

void do_split(blockheader *bhdr, size_t bhdr_payload_size);

blockheader *do_coalesce_right(blockheader *bhdr);

#define ALIGN(val)  \
    (((val) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))

#define AL_ARENAHDR_SIZE ALIGN(sizeof(arenaheader))

#define AL_BLOCKHDR_SIZE ALIGN(sizeof(blockheader))

#define GET_ARENAHDR(ptr) (arenaheader *)ALIGN((uintptr_t)ptr)

#define GET_BLOCKHDR(ptr) (blockheader *)((uintptr_t)ptr - AL_BLOCKHDR_SIZE)

#define MIN_BLOCK_SIZE (AL_BLOCKHDR_SIZE + ALIGN(sizeof(char)))

#define MIN(a, b) ((a) < (b) ? (a) : (b))

#endif /* DMALLOC_INTERNALS_H */
