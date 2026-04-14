/*
*       dmalloc
*
*   Project repository at https://github.com/sizeof-dario/dmalloc.git.
*
*   You can read the documentation at https://sizeof-dario.github.io/dmalloc/.
*
*******************************************************************************/

/*      "dmalloc_alloc.h"                                                     */

#ifndef DMALLOC_ALLOC_H
#define DMALLOC_ALLOC_H 1

#ifndef _DEFAULT_SOURCE
 /* Feature test macro required for sbrk(). */
 #define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "dmalloc_defs.h"

/*  Performs block spliting on the block whose header is pointed to by `bhdr`.
    `size` determines how much space must remain in the block that is split.
    No check is done about if block splitting can happen. */
void do_split(blockheader *bhdr, size_t size);

void try_split(blockheader *bhdr, size_t bhdr_payload_size);


/*  Allocates `size` bytes of memory on the arena whose header is pointed to by
    `ahdr` or on the heap if `ahdr` is NULL. It's not thread-safe. */
void *dmalloc_unlocked(size_t size, arenaheader *ahdr);



/*  Performs block coalescing on the right on the block whose header is pointed
    to by `bhdr`. No check is done about if block coalescing can happen. */
blockheader *do_coalesce_right(blockheader *bhdr);

blockheader *try_coalesce(blockheader *bhdr);

/*  Frees memory pointed to by `p` fro, the arena whose header is pointed to by
    `ahdr`. If `ahdr` is NULL, memory is freed from the heap. */
void dfree_unlocked(void *p, arenaheader *ahdr);

#endif /* DMALLOC_ALLOC_H */
