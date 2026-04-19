/*  dmalloc - A didactic memory allocator for UNIX systems.

Project repository at https://github.com/sizeof-dario/dmalloc.git.
Documentation is avaiable at https://sizeof-dario.github.io/dmalloc/. 

Latest change on date 2026-04-15. */

/**
 *  Arenas-related functions "dmalloc_arenas.h".
 */

#ifndef DMALLOC_ARENAS_H
#define DMALLOC_ARENAS_H 1

#include <errno.h>
#include <unistd.h>

#include "dmalloc.h"
#include "dmalloc_defs.h"

/*  Wrapper for `sbrk()`. It's used to make `sbrk()` compatible with the 
    `brkshifter` field in the `arenaheader` struct. A call to this function is
    equivalent to `sbrk(delta)`, 'unused` is ignored. */
void  *heapsbrk(intptr_t delta, arenaheader *unused);

/*  Changes of `delta` bytes the position of the break in the arena whose header
    is pointed to by `ahdr`. */
void *arenasbrk(intptr_t delta, arenaheader *ahdr);

/*  Initializes the memory region pointed to by `backing_memory` and of size
    `capacity` to an arena. If `backing_memory` is NULL, the heap is
    initialized. 
    
    This function is MT-unsafe. */
int bckminit(void *backing_memory, size_t capacity);

/*  Initializes the heap as an arena.

    This function is MT-unsafe. */
int heapinit_unlocked();

/*  MT-safe wrapper for `heapinit_unlocked()`. */
int heapinit();

/*  darenainit() and darenadestroy() are part of the arenas-related functions.
    Thier signatures, however, can be found in "dmalloc.h" for them being part
    of the allocator API. */

#endif /* DMALLOC_ARENAS_H */