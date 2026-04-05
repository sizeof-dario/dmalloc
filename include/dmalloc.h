/******************************************************************************
    
        dmalloc hybrid arena allocator.

    Project repo at https://github.com/sizeof-dario/dmalloc.git.

******************************************************************************/

/*  "dmalloc.h" - Master include file.

    Contains all the API definitions.   */

#ifndef DMALLOC_H
#define DMALLOC_H 1

/*  #### DESCRIPTION
    `arenainit()` initializes to an arena the memory region pointed to by 
    `backing_memory` and of size `capacity`.
    #### RETURN VALUE
    On success, returns `0`.
    On failure, returns `-1` and sets `errno`.
    #### ERRORS
    -   `ENOMEM` 
        Not enough space. `arenainit()` failed because `capacity` was not big
        enough to accomodate for the arena header.
    */
int arenainit(void *backing_memory, size_t capacity);



/*  #### DESCRIPTION
    `dmalloc()` allocates `size` bytes of memory in the arena pointed to by 
    `arena` or on the heap if `arena` is `NULL`.
    #### RETURN VALUE
    On success, returns a pointer to the allocated memory.
    On failure, returns `NULL` and sets `errno`.
    #### ERRORS
    -   `ENOMEM` 
        Out of memory. Possibly, `dmalloc()` was called upon an arena with not 
        enough available space. More unlikely, other reasons could be that an 
        error occurred while the heap was being initialized or that the heap is
        full. 
    #### STANDARDS
    -   `dmalloc(0, arena)` 
        Trying to allocate `0` bytes is considered a valid operation, so 
        `dmalloc()` doesn't fail and still returns an unique pointer as usual. 
        However, such pointer shall never be dereferenced, that being UB, and
        can, and should, be passed to `dfree()` for cleanup.
    #### EXAMPLES
    The following code shows how `dmalloc()` can be used with an arena on the
    stack. In particular, some backing memory of `BM_CAPACITY` is defined,
    `arenainit()` is called to initialized it, and its return value is checked
    to be sure initialization succeded, then 'dmalloc()` gets called, its 
    return value is checked as well. After that, and only after that, some
    values are assigned to the allocated memory to eventually be printed.

    ```
    #include <stdio.h>
    #include <stdlib.h>
    #include "dmalloc.h"

    #define BM_CAPACITY 1024

    int main()
    {
        char backing_memory[BM_CAPACITY];

        int some_integers[] = {-123, 45, -678, 90};

        if(arenainit(backing_memory, BM_CAPACITY) < 0)
        {
            perror("arenainit");
            exit(EXIT_FAILURE);
        }

        int *p = (int *)dmalloc(4 * sizeof(int), backing_memory);

        if(p == NULL)
        {
            perror("dmalloc");
            exit(EXIT_FAILURE);
        }

        for(int i = 0; i < 4; i++)
        {
            p[i] = some_integers[i];
        }

        for(int j = 0; j < 4; j++)
        {
            printf("%d\n", p[j]);
        }

        dfree(p, backing_memory);
    }
    ``` */
void *dmalloc(size_t size, void *arena);



/*  #### DESCRIPTION
    `dfree()` deallocates memory pointed to by `p` from the arena pointed to by
    `arena` or from the heap, if `arena` is `NULL`. If `p` is `NULL`, no action
    shall occurr. The memory pointed to by `p` must have been previously 
    allocated by an allocator function and, if it does, it must not have been 
    already deallocated. Otherwise, in both cases, it's UB.
    #### RETURN VALUE
    `dfree()` shall return no value, and preserves `errno`.
    #### ERRORS
    No errors are defined.
    #### NOTES
    ISO/IEC 9899:2024, §7.24.3.3, ¶2 establishes the UB cases for `free()`. By
    default, `dfree()` follows the standard. However, for a slower but safer
    version of the function that attempts checking if some memory was already 
    freed or if a pointer wasn't returned by any allocator function, a 
    non-standard pedantic version exists. It can be enabled via the allocator 
    CMake build system, with the `-DENABLE_DFREE_PEDANTIC=ON` flag.
    */
void dfree(void *p, void *arena);



/*  #### DESCRIPTION
    `dcalloc()` allocates memory for `n` elements of `size` bytes in the arena
    pointed to by `arena` or on the heap if `arena` is `NULL`. The memory is
    initialized to `0`.
    #### RETURN VALUE
    On success, returns a pointer to the allocated memory.
    On failure, returns `NULL` and sets `errno`.
    #### ERRORS
    -   `ENOMEM` 
        Out of memory. Possibly, `dcalloc()` was called upon an arena with not 
        enough available space. More unlikely, other reasons could be that an 
        integer overflow would occur from the multiplication `n * size`, or 
        that an error occurred while the heap was being initialized or that the
        heap is full. 
    #### STANDARDS
    -   `dcalloc(0, size, arena)` 
        Trying to allocate `0` elements of any size is considered a valid
        operation, so `dcalloc()` doesn't fail and still returns an unique 
        pointer as usual. However, such pointer shall never be dereferenced, 
        that being UB, and can, and should, be passed to `dfree()` for cleanup.
    -   `dcalloc(n, 0, arena)`    
        Trying to allocate any number of elements of size `0` results in the
        same scenario as `dcalloc(0, size, arena)`. */
void *dcalloc(size_t n, size_t size, void *arena);



/*
    #### DESCRIPTION
    `drealloc()` reallocates memory pointed to by `p` from the arena pointed to
    by `src` to the arena pointed to by `dest`, possibly resizing it to `size`.
    `drealloc()` also reallocates on or from the heap if, respectively, `dest`
    or `src` are `NULL`.
    #### RETURN VALUE
    On success, returns a pointer to the reallocated memory.
    On failure, returns `NULL` and sets `errno`.
    #### ERRORS
    -   `ENOMEM`
    Out of memory. Possibly, `dmalloc()` was called upon an arena with not 
        enough available space. More unlikely, other reasons could be that an 
        error occurred while the heap was being initialized or that the heap is
        full.
    #### NOTES
    If `p` isn't `NULL` and doesn't match any pointer returned by any allocator
    functions, the behaviour is undefined.
    #### STANDARDS
    -   `drealloc(NULL, size, dest, src)`
        If `p` is `NULL`, `drealloc()` behaves like `dmalloc(size, dest)`.
    -   `drealloc(p, 0, dest, src)`
        If `size` is `0`, `drealloc()` behaves like `dfree(p, src)`. The
        function still returns `p`, so the caller must make sure to never
        dereference it.
    #### EXAMPLES
    The following code shows ho `drealloc()` can be used to reallocate some
    memory from one arena to another.

    ```
    #include <stdio.h>
    #include <stdlib.h>
    #include "dmalloc.h"

    #define BM_CAPACITY 1024

    int main()
    {
        char arena_1[BM_CAPACITY];
        char arena_2[BM_CAPACITY];

        if(arenainit(arena_1, BM_CAPACITY) < 0 
        || arenainit(arena_2, BM_CAPACITY) < 0)
        {
            perror("arenainit");
            exit(EXIT_FAILURE);
        }

        double *p = (double *)dmalloc(2 * sizeof(double), arena_1);
        
        if(p == NULL)
        {
            perror("dmalloc");
            exit(EXIT_FAILURE);
        }

        p[0] = 3.141593;
        p[1] = 2.718282;

        double *q = drealloc(p, 2 * sizeof(double), arena_2, arena_1);

        if(q == NULL)
        {
            perror("dcalloc");
            exit(EXIT_FAILURE);        
        }

        printf("%f\n%f\n", q[0], q[1]);

        dfree(q, arena_2);
    }
    ```
*/
void *drealloc(void *p, size_t size, void *dest, void *src);



/*
    #### DESCRIPTION
    #### RETURN VALUE
    #### ERRORS
    #### NOTES
    #### STANDARDS
    #### EXAMPLES
*/
void *dreallocarray(void *p, size_t n, size_t size, void *dest, void *src);



#endif  /* DMALLOC_H */
