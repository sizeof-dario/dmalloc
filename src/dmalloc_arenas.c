/*  dmalloc - A didactic memory allocator for UNIX systems.

Project repository at https://github.com/sizeof-dario/dmalloc.git.
Documentation is avaiable at https://sizeof-dario.github.io/dmalloc/. 

Latest change on date 2026-04-15. */

/**
 *  IMplementations of arenas-related functions "dmalloc_arenas.c".
 */

#include "dmalloc_arenas.h"

pthread_mutex_t heapinit_lock = PTHREAD_MUTEX_INITIALIZER;
void *heap_start = NULL;

void* heapsbrk(intptr_t delta, arenaheader *unused)
{
    (void)(unused);
    return sbrk(delta);
}

void *arenasbrk(intptr_t delta, arenaheader *ahdr)
{
    if(ahdr == NULL)
    {
        return NULL;
    }

    if(delta == 0)
    {
        return ahdr->arena_brk;
    }

    uintptr_t old_brk = (uintptr_t)ahdr->arena_brk;
    uintptr_t new_brk = delta > 0 ? old_brk + (uintptr_t)( delta)
                                  : old_brk - (uintptr_t)(-delta);

    if(delta > 0 ? new_brk <= old_brk : new_brk >= old_brk)
    {
        errno = ENOMEM;
        return (void *)(-1);
    }

    if(new_brk < (uintptr_t)ahdr->arena_start 
    || new_brk > (uintptr_t)ahdr->arena_end)
    {
        errno = ENOMEM;
        return (void *)(-1);
    }

    ahdr->arena_brk = (void *)new_brk;
    return (void *)old_brk;
}

int bckminit(void *backing_memory, size_t capacity)
{
    void *(*brkshifter)(intptr_t, arenaheader *) = arenasbrk;
    
    if(backing_memory == NULL)
    {
        heap_start = sbrk(0);
        backing_memory = heap_start;
        capacity = SIZE_MAX;
        brkshifter = heapsbrk;
    }

    arenaheader *ahdr = GET_ARENAHDR(backing_memory);
    ptrdiff_t padding = (uintptr_t)ahdr - (uintptr_t)backing_memory;

    if(backing_memory == heap_start)
    {
        if(sbrk(padding + AL_ARENAHDR_SIZE) == (void *)(-1))
        {
            return -1;
        }
    }
    else
    {
        if(padding + AL_ARENAHDR_SIZE > capacity)
        {
            errno = ENOMEM;
            return -1;
        }
    }

    capacity -= padding + AL_ARENAHDR_SIZE;

    int retval = pthread_mutex_init(&ahdr->lock, NULL);
    if(retval != 0)
    {
        errno = retval;
        return -1;
    }
    
    ahdr->arena_start = (void *)((uintptr_t)ahdr + AL_ARENAHDR_SIZE);
    ahdr->arena_brk = ahdr->arena_start;
    ahdr->arena_end = (void *)((uintptr_t)ahdr->arena_start + capacity);
    ahdr->brkshifter = brkshifter;
    ahdr->bhdr_first = NULL;

    return 0;
}

int heapinit_unlocked()
{
    return bckminit(NULL, SIZE_MAX);
}

int heapinit()
{
    if(heap_start == NULL)
    {
        int retval = pthread_mutex_lock(&heapinit_lock);
        if(retval != 0)
        {
            errno = retval;
            return -1;
        }

        if(heap_start == NULL)
        {
            if(heapinit_unlocked() < 0)
            {
                retval = pthread_mutex_unlock(&heapinit_lock);
                if(retval != 0)
                {
                    errno = retval;
                }
                return -1;
            }
        }

        retval = pthread_mutex_unlock(&heapinit_lock);
        if(retval != 0)
        {
            errno = retval;
            return -1;
        }
    }
    return 0;
}

int darenainit(void *backing_memory, size_t capacity)
{
    if(backing_memory == NULL)
    {
        return 0;
    }

    return bckminit(backing_memory, capacity);
}

int darenadestroy(darena_t *arena)
{
    if(arena == NULL)
    {
        return 0;
    }

    arenaheader *ahdr = GET_ARENAHDR(arena);

    int retval = pthread_mutex_trylock(&ahdr->lock);
    if(retval != 0)
    {
        errno = retval;
        return -1;
    }

    retval = pthread_mutex_unlock(&ahdr->lock);
    if(retval != 0)
    {
        errno = retval;
        return -1;
    }

    retval = pthread_mutex_destroy(&ahdr->lock);
    if(retval != 0)
    {
        errno = retval;
        return -1;
    }

    ahdr->arena_start = NULL;
    ahdr->arena_brk   = NULL;
    ahdr->arena_end   = NULL;
    ahdr->brkshifter  = NULL;
    ahdr->bhdr_first  = NULL;

    return 0;
}
