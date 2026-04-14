/*
*       dmalloc
*
*   Project repository at https://github.com/sizeof-dario/dmalloc.git.
*
*   You can read the documentation at https://sizeof-dario.github.io/dmalloc/.
*
*******************************************************************************/

/*      "dmalloc_arenas.h"                                                    
*
*   Contains function signatures for the arena-related functions. darenainit()
*   and darenadestroy() signatures are in "dmalloc.h" because they are part of 
*   the library API.                                                          */

#ifndef DMALLOC_ARENAS_H
#define DMALLOC_ARENAS_H 1

#include <errno.h>
#include <unistd.h>
#include "dmalloc.h"
#include "dmalloc_defs.h"

void *sbrk_wrap(intptr_t delta, arenaheader *unused);
void *arenasbrk(intptr_t delta, arenaheader *ahdr);

int backmeminit(void *backing_memory, size_t capacity);
int heapinit();
int ensureheap();

#endif /* DMALLOC_ARENAS_H */