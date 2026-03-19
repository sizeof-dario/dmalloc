/*
    dmalloc memory allocator

    Check the project repo at https://github.com/sizeof-dario/dmalloc.git for
    more information about it.

******************************************************************************/

/*  "dmalloc.h" - Master include file for the dmalloc memory allocator.

    Contains all the API definitions.                                        */

#ifndef DMALLOC_H
#define DMALLOC_H 1

#include <assert.h>
#include <stdalign.h>
#include <stddef.h>
#include <string.h>



/*  #### DESCRIPTION
    `arenainit()` initializes the buffer pointed to by `buffer` and of size 
    `size`  to an arena.
    #### RETURN VALUE
    On success, returns `0`.
    On failure, returns `-1` and sets `errno`.
    #### ERRORS
    -   `EINVAL` 
        Invalid argument. Either `buffer` was `NULL` or `size` was `0`.
    -   `ENOMEM` 
        Not enough space. `size` was not big enough for the arena header.    */
int arenainit(void *buffer, size_t size);



void *dmalloc(size_t size, void *arena);

void dfree(void *p, void *arena);

void *dcalloc(size_t n, size_t size, void *arena);

void *drealloc(void *p, size_t size, void *dest, void *src);

void *dreallocarray(void *p, size_t n, size_t size, void *dest, void *src);



#endif  /* DMALLOC_H */