/*  dmalloc - A didactic memory allocator for UNIX systems.

Project repository at https://github.com/sizeof-dario/dmalloc.git.
Documentation is avaiable at https://sizeof-dario.github.io/dmalloc/. 

Latest change on date 2026-04-15. */

/**
 *  Allocator core functions "dmalloc_core.h".
 */

#ifndef DMALLOC_CORE_H
#define DMALLOC_CORE_H 1

#ifndef _DEFAULT_SOURCE
 /* Feature test macro required for sbrk(). */
 #define _DEFAULT_SOURCE
#endif

#include <errno.h>
#include <string.h>
#include <unistd.h>

#include "dmalloc_defs.h"

void do_split(blockheader *bhdr, size_t size);
void try_split(blockheader *bhdr, size_t bhdr_payload_size);

void *dmalloc_unlocked(size_t size, arenaheader *ahdr);



blockheader *do_coalesce_right(blockheader *bhdr);
blockheader *try_coalesce(blockheader *bhdr);

void dfree_unlocked(void *p, arenaheader *ahdr);

#endif /* DMALLOC_CORE_H */
