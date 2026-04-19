/*  dmalloc - A didactic memory allocator for UNIX systems.

Project repository at https://github.com/sizeof-dario/dmalloc.git.
Documentation is avaiable at https://sizeof-dario.github.io/dmalloc/. 

Latest change on date 2026-04-14. */

/**
 *  Definitions of types and macros "dmalloc_defs.h".
 */

#ifndef DMALLOC_DEFS_H
#define DMALLOC_DEFS_H 1

#include <stdalign.h>
#include <stddef.h>
#include <stdint.h>
#include <pthread.h>

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

#define ALIGN(x) (((x) + alignof(max_align_t)-1) & ~(alignof(max_align_t)-1))

#define AL_ARENAHDR_SIZE ALIGN(sizeof(arenaheader))
#define AL_BLOCKHDR_SIZE ALIGN(sizeof(blockheader))

#define MIN_BLOCK_SIZE (AL_BLOCKHDR_SIZE + ALIGN(1))

#define GET_ARENAHDR(p) (arenaheader *)ALIGN((uintptr_t)(p))
#define GET_BLOCKHDR(p) (blockheader *)((uintptr_t)(p) - AL_BLOCKHDR_SIZE)

#endif /* DMALLOC_DEFS_H */