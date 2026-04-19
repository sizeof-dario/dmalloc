/*  dmalloc - A didactic memory allocator for UNIX systems.

Project repository at https://github.com/sizeof-dario/dmalloc.git.
Documentation is avaiable at https://sizeof-dario.github.io/dmalloc/. 

Latest change on date 2026-04-18. */

/**
 *  Functions for debugging "dmalloc_debug.h".
 */

#include "dmalloc_defs.h"
#include <string.h>
#include <unistd.h>

void ultos(unsigned long ul, char *buf, size_t bufsize, unsigned char base, int is_ptr);

ssize_t writeln(int fd, const void* buf, size_t n);

void arenadump(void *p);

