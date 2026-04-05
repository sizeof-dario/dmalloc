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
    void               *arena_start;
    void               *arena_brk;
    void               *arena_end;
    blockheader        *bhdr_first;
    void             *(*brkshifter)(intptr_t, struct arenaheader*);
} arenaheader;



/*  #### DESCRIPTION
    `heapinit()` initializes the heap as an arena.
    #### RETURN VALUE
    On success, returns `0`.
    On failure, returns `-1` with `errno` set.
    #### ERRORS
    -   `ENOMEM`
        Out of memory. `heapinit()` called `sbrk()` that failed.  
*/
int heapinit();



/*  #### DESCRIPTION
    `arenasbrk()` increases or decreases of `delta` bytes the break of the
    arena pointed to by `ahdr`.
    #### RETURN VALUE
    On success, returns the old arena break.
    On failure, returns `(void *)(-1)` and sets `errno`.
    #### ERRORS
    -   `EINVAL`
        Invalid argument. The function received `NULL` as the value for `ahdr`.
    -   `EOVERFLOW`
        Value too large for defined data type. `delta` was too big in modulus.
    -   `ERANGE`
        Result too large. The shift would cause the break to point somewhere
        outside the arena.
    #### NOTES
    `arenasbrk(0, ahdr)` can be used to obtain the current break value. */
void *arenasbrk(intptr_t delta, arenaheader *ahdr);



void do_split(blockheader *bhdr, size_t bhdr_payload_size);

blockheader *do_coalesce_right(blockheader *bhdr);

#define ALIGN(val)  \
    (((val) + alignof(max_align_t) - 1) & ~(alignof(max_align_t) - 1))

#define AL_ARENAHDR_SIZE ALIGN(sizeof(arenaheader))

#define AL_BLOCKHDR_SIZE ALIGN(sizeof(blockheader))

#define GET_ARENAHDR(ptr) (arenaheader *)ALIGN((uintptr_t)ptr)

#define GET_BLOCKHDR(ptr) (blockheader *)((uintptr_t)ptr - AL_BLOCKHDR_SIZE)

#define MIN_BLOCK_SIZE (AL_BLOCKHDR_SIZE + ALIGN(sizeof(char)))



#endif /* DMALLOC_INTERNALS_H */
