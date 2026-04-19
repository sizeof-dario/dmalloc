/*  dmalloc - A didactic memory allocator for UNIX systems.

Project repository at https://github.com/sizeof-dario/dmalloc.git.
Documentation is avaiable at https://sizeof-dario.github.io/dmalloc/. 

Latest change on date 2026-04-18. */

/**
 *  Functions for debugging impelentation "dmalloc_debug.h".
 */

#include "dmalloc_debug.h"

extern void *heap_start; /* Found in "dmalloc_arenas.c". */

void ultos(unsigned long ul, char *buf, size_t bufsize, unsigned char base, int is_ptr)
{
    memset(buf, '\0', bufsize);

    if(base != 10 && base != 16)
    {
        return;
    }

    size_t shift = 0;

    if(base == 16)
    {
        buf[0] = '0';
        buf[1] = 'x';
        shift = 2;
    }

    if(ul == 0)
    {
        if(is_ptr)
        {
            buf[0] = 'N';
            buf[1] = 'U';
            buf[2] = 'L';
            buf[3] = 'L';
        }
        else
        {
            buf[shift] = '0';
        }

        return;
    }

    size_t len = 0;

    unsigned long ul_temp = ul;
    while (ul_temp)
    {
        ul_temp /= base;
        len++;
    }
    
    for(size_t i = 0; i < len; i++)
    {
        buf[shift + (len - i - 1)] = (ul % base) < 10 ? ('0' + (ul % base)) : ('a' + (ul % base - 10));
        ul /= base;
    }
}

void arenadump(void *p)
{
    if(p == NULL)
    {
        p = heap_start;
    }

    arenaheader *ahdr = GET_ARENAHDR(p);
    pthread_mutex_lock(&ahdr->lock);

    const size_t BUFSIZE = 100;
    char buffer[BUFSIZE];

    write(1, "ARENA DUMP\n", 11);

    write(1, "arena_start = ", 14);
    ultos((unsigned long)ahdr->arena_start, buffer, BUFSIZE, 16, 1);
    write(1, buffer, BUFSIZE);
    write(1, "\n", 1);

    write(1, "arena_brk   = ", 14);
    ultos((unsigned long)ahdr->arena_brk, buffer, BUFSIZE, 16, 1);
    write(1, buffer, BUFSIZE);
    write(1, "\n", 1);

    write(1, "arena_end   = ", 14);
    ultos((unsigned long)ahdr->arena_end, buffer, BUFSIZE, 16, 1);
    write(1, buffer, BUFSIZE);
    write(1, "\n", 1);

    write(1, "bhdr_first  = ", 14);
    ultos((unsigned long)ahdr->bhdr_first, buffer, BUFSIZE, 16, 1);
    write(1, buffer, BUFSIZE);
    write(1, "\n", 1);

    blockheader *bhdr = ahdr->bhdr_first;

    unsigned long blockcounter = 1;
    while (bhdr != NULL)
    {
        write(1, "------------------------------------------------\n", 49);
        ultos(blockcounter, buffer, BUFSIZE, 10, 0);
        write(1, buffer, BUFSIZE);
        write(1, ":THIS address = ", 16);
        ultos((unsigned long)bhdr, buffer, BUFSIZE, 16, 1);
        write(1, buffer, BUFSIZE);
        write(1, "\n", 1);

        ultos(blockcounter, buffer, BUFSIZE, 10, 0);
        write(1, buffer, BUFSIZE);
        write(1, ":payload_size = ", 16);
        ultos((unsigned long)bhdr->payload_size, buffer, BUFSIZE, 10, 0);
        write(1, buffer, BUFSIZE);
        write(1, "\n", 1);

        ultos(blockcounter, buffer, BUFSIZE, 10, 0);
        write(1, buffer, BUFSIZE);
        write(1, ":is_free      = ", 16);
        ultos((unsigned long)bhdr->is_free, buffer, BUFSIZE, 10, 0);
        write(1, buffer, BUFSIZE);
        write(1, "\n", 1);

        ultos(blockcounter, buffer, BUFSIZE, 10, 0);
        write(1, buffer, BUFSIZE);
        write(1, ":bhdr_prev    = ", 16);
        ultos((unsigned long)bhdr->bhdr_prev, buffer, BUFSIZE, 16, 1);
        write(1, buffer, BUFSIZE);
        write(1, "\n", 1);

        ultos(blockcounter, buffer, BUFSIZE, 10, 0);
        write(1, buffer, BUFSIZE);
        write(1, ":bhdr_next    = ", 16);
        ultos((unsigned long)bhdr->bhdr_next, buffer, BUFSIZE, 16, 1);
        write(1, buffer, BUFSIZE);
        write(1, "\n", 1);

        bhdr = bhdr->bhdr_next;
        blockcounter++;
    }
    
    write(1, "------------------------------------------------\n\n", 50);

    pthread_mutex_unlock(&ahdr->lock);
}


