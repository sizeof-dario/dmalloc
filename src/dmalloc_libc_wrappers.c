/*  dmalloc - A didactic memory allocator for UNIX systems.

Project repository at https://github.com/sizeof-dario/dmalloc.git.
Documentation is avaiable at https://sizeof-dario.github.io/dmalloc/. 

Latest change on date 2026-04-15. */

/**
 *  Function wrappers for libc malloc functions "dmalloc_libc_wrappers.c"
 */

#include "dmalloc.h"
#include <sys/types.h>
#include <unistd.h>

void *malloc(size_t size)
{
    return dmalloc(size, NULL);
}

void free(void *p)
{
    dfree(p, NULL);
}

void *calloc(size_t n, size_t size)
{
    return dcalloc(n, size, NULL);
}

void *realloc(void *p, size_t size)
{
    return drealloc(p, size, NULL, NULL);
}

void *reallocarray(void *p, size_t n, size_t size)
{
    return dreallocarray(p, n, size, NULL, NULL);
}
