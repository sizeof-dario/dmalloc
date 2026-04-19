/*  dmalloc - A didactic memory allocator for UNIX systems.

Project repository at https://github.com/sizeof-dario/dmalloc.git.
Documentation is avaiable at https://sizeof-dario.github.io/dmalloc/. 

Latest change on date 2026-04-14. */

/**
 *  dmalloc API definitions "dmalloc.h".
 */

#ifndef DMALLOC_H
#define DMALLOC_H 1

#include <stddef.h> /* For size_t. */

/*  Pointers to this type point to memory initialized by `darenainit()`. */
typedef void darena_t;

/**
 *  ARENAS-RELATED FUNCTIONS
 */

/*  Initializes the memory region pointed to by `backing_memory` and of size
    `capacity` to an arena. */
int darenainit(void *backing_memory, size_t capacity);

/*  Destroys the arena pointed to by `arena`. */
int darenadestroy(darena_t *arena);

/**
 *  CORE FUNCTIONS
 */

/*  Allocates `size` bytes of memory in the arena pointed to by `arena`. If
    `arena` is NULL, the memory is allocated on the heap. */
void *dmalloc(size_t size, darena_t *arena);

/*  Deallocates the memory pointed to by `p` from the arena pointed to by
    `arena`. If `arena` is NULL, the memory is deallocated from the heap. */
void dfree(void *p, darena_t *arena);

/*  Allocates memory for `n` objects of size `size` in the arena pointed to by
    `arena` and initializes it to 0. If `arena` is NULL, the memory is allocated
    on the heap. */
void *dcalloc(size_t n, size_t size, darena_t *arena);

/*  Reallocates the memory pointed to by `p` from `src` to a new block of size
    `size` in `dest`. Both `src` and `dest` can either be NULL for the memory
    being reallocated, respectively, from or on the heap. */
void *drealloc(void *p, size_t size, darena_t *dest, darena_t *src);

/*  Reallocates the memory pointed to by `p` from `src` to a block in `dest`
    that is big enough for `n` objects of size `size`. Both `src` and `dest` can
    be NULL for the memory to be reallocated from or on the heap. */
void *dreallocarray(void *p, size_t n, size_t size, darena_t *dest, 
                    darena_t *src);
                    
#endif /* DMALLOC_H. */
