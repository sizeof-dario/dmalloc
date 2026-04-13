/*
*       dmalloc allocator.
*
*   Project repository at https://github.com/sizeof-dario/dmalloc.git.
*
*   You can read the documentation at https://sizeof-dario.github.io/dmalloc/.
*
*******************************************************************************/

/*      "dmalloc_libc_wrappers.c"                                             */

#include "dmalloc.h"

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
