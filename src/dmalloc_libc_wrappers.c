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

void free(void *__ptr)
{
    dfree(__ptr, NULL);
}

void *calloc(size_t __nmemb, size_t __size)
{
    return dcalloc(__nmemb, __size, NULL);
}

void *realloc(void *__ptr, size_t __size)
{
    return drealloc(__ptr, __size, NULL, NULL);
}

void *reallocarray(void *__ptr, size_t __nmemb, size_t __size)
{
    return dreallocarray(__ptr, __nmemb, __size, NULL, NULL);
}
