/*
    dmalloc memory allocator

    Check the project repo at https://github.com/sizeof-dario/dmalloc.git for
    more information about it.

******************************************************************************/

/* TODO: drealloc() is a bit nasty it he reallocation of a block, when it must 
    be shrunk and it's at the end of th list. A more clean way to do it should 
    be found. */

#include "dmalloc_internals.h"
#include "dmalloc.h"

#include "stdio.h"

static void *heap_start = NULL;



/*  LCOV_EXCL_      START. */

static inline void* sbrk_wrap(intptr_t delta, arenaheader *dummy)
{
    UNUSED(dummy);
    return sbrk(delta);
}



int heapinit()
{

    arenaheader *hhdr = GET_ARENAHDR(heap_start);
    ptrdiff_t padding = (uintptr_t)hhdr - (uintptr_t)heap_start;

    if(sbrk(padding + AL_ARENAHDR_SIZE) == (void *)(-1))
    {
        return -1;
    }

    /*  WE can think of the heap as backing memory of SIZE_MAX capacity. That's
        way past the real limit, but it allows us to unify the code. */

    hhdr->arena_start    = (void *)((uintptr_t)hhdr + AL_ARENAHDR_SIZE);
    hhdr->arena_brk      = hhdr->arena_start;
    hhdr->arena_end      = (void *)((uintptr_t)hhdr->arena_start + SIZE_MAX);
    hhdr->bhdr_first     = NULL;
    hhdr->brkshifter     = sbrk_wrap;

    return 0;
}

/*  LCOV_EXCL_      STOP. */



void *arenasbrk(intptr_t delta, arenaheader *ahdr)
{
    if(ahdr == NULL)
    {
        errno = EINVAL;
        return (void *)(-1);
    }

    uintptr_t old_brk = (uintptr_t)ahdr->arena_brk;

    if(delta == 0)
    {
        return (void *)old_brk;
    }

    uintptr_t new_brk = delta > 0 
                    ? old_brk + (uintptr_t)( delta) 
                    : old_brk - (uintptr_t)(-delta);

    if (delta > 0 ? new_brk <= old_brk : new_brk >= old_brk)
    {
        errno = EOVERFLOW;
        return (void *)(-1);
    }

    if(new_brk < (uintptr_t)ahdr->arena_start 
    || new_brk > (uintptr_t)ahdr->arena_end)
    {
        errno = ERANGE;
        return (void *)(-1);
    }

    ahdr->arena_brk = (void *)new_brk;

    return (void *)old_brk;
}



int arenainit(void *backing_memory, size_t capacity)
{
    if(backing_memory == NULL)
    {
        return 0;
    }

    arenaheader *ahdr = GET_ARENAHDR(backing_memory);

    ptrdiff_t padding = (uintptr_t)ahdr - (uintptr_t)backing_memory;

    if(padding + AL_ARENAHDR_SIZE > capacity)
    {
        errno = ENOMEM;
        return -1;
    }

    capacity -= padding + AL_ARENAHDR_SIZE;

    ahdr->arena_start    = (void *)((uintptr_t)ahdr + AL_ARENAHDR_SIZE);
    ahdr->arena_brk      = ahdr->arena_start;
    ahdr->arena_end      = (void *)((uintptr_t)ahdr->arena_start + capacity);
    ahdr->bhdr_first     = NULL;
    ahdr->brkshifter     = arenasbrk;

    return 0;
}



void do_split(blockheader *bhdr, size_t bhdr_payload_size)
{
    /*  We need to save the current data to later update the headers of the 
        blocks adjacent to the one we're about to split and initialize the 
        header of the extra block that will be created by the splitting.    */

    size_t payload_size_old = bhdr->payload_size;
    blockheader *bhdr_next_old = bhdr->bhdr_next;

    /*  We create a new header.                 */

    blockheader *bhdr_new = (blockheader *)
        ((uintptr_t)bhdr + AL_BLOCKHDR_SIZE + bhdr_payload_size);

    /*  We update the splitted block header.    */
    bhdr->payload_size = bhdr_payload_size;
    bhdr->bhdr_next = bhdr_new;

    /*  We initialize the new block header.     */
    bhdr_new->payload_size 
        = payload_size_old - bhdr_payload_size - AL_BLOCKHDR_SIZE;
    bhdr_new->is_free = 1;
    bhdr_new->bhdr_prev = bhdr;
    bhdr_new->bhdr_next = bhdr_next_old;

    /*  We update the header of the block that next to the new one. */
    
    /*  The branch is always taken due to dfree() always lowering the break 
        when the last block in list is freed.   */
    /*  LCOV_EXCL_      START. */
    if(bhdr_next_old != NULL)
    {
        bhdr_next_old->bhdr_prev = bhdr_new;
    }
    /*  LCOV_EXCL_      STOP. */

}



static inline void try_split(blockheader *bhdr, size_t bhdr_payload_size)
{
    /*  If the space we need is small enough for a non-degenerate block to fit
        in what's left of the free block after the allocation, we can perform 
        block splitting.    */
    if((bhdr->payload_size - bhdr_payload_size) >= MIN_BLOCK_SIZE)
    {
        do_split(bhdr, bhdr_payload_size);
    }
}



void *dmalloc(size_t size, void *arena)
{
    /*  LCOV_       EXCL_START. */

    if(arena == NULL && heap_start == NULL)
    {
        heap_start = sbrk(0);
        if(heapinit() < 0)
        {
            errno = ENOMEM;
            return NULL;
        }
    }

    if(arena == NULL)
    {
        arena = heap_start;
    }

    /*  LCOV_EXCL_      STOP. */





    arenaheader *ahdr = GET_ARENAHDR(arena); 

    size = ALIGN(size);





    /*  To allocate the memory, we first assume some free allocated blocks 
        already exist; thus, we traverse their list to try finding one with a 
        big enough payload size for reusage. */

    blockheader *bhdr_start = ahdr->bhdr_first;
    blockheader *bhdr_curr  = bhdr_start;
    blockheader *bhdr_last  = NULL;                   

    while(bhdr_start != NULL && bhdr_curr != NULL)
    {
        if(bhdr_curr->is_free == 1 && (bhdr_curr->payload_size >= size))
        {
            bhdr_curr->is_free = 0;

            try_split(bhdr_curr, size);

            return (void *)((uintptr_t)bhdr_curr + AL_BLOCKHDR_SIZE);
        }

        /*  We keep track of the last accessed header so that if we exit the
            loop without returning, meaning we reached the end of the list,
            we can create a new block and link its header to it.            */    
        bhdr_last = bhdr_curr;

        bhdr_curr = bhdr_curr->bhdr_next;
    }

    /*  If we weren't able to find a suitable block, or if there were no blocks
        to begin with, we need to raise the break and create a new block. */
    blockheader *p = ahdr->brkshifter(size + AL_BLOCKHDR_SIZE, ahdr);
    
    if(p == (void *)(-1))
    {
        errno = ENOMEM;
        return NULL;
    }

    if(ahdr->bhdr_first == NULL)
    {
        ahdr->bhdr_first = p;
    }

    p->is_free         = 0;
    p->payload_size    = size;
    p->bhdr_prev       = bhdr_last;
    p->bhdr_next       = NULL;

    if(bhdr_last != NULL)
    {
        p->bhdr_prev->bhdr_next = p;
    }

    return (void *)((uintptr_t)p + AL_BLOCKHDR_SIZE);
}



blockheader *do_coalesce_right(blockheader *bhdr)
{
    bhdr->payload_size += (AL_BLOCKHDR_SIZE + bhdr->bhdr_next->payload_size);

    bhdr->bhdr_next = bhdr->bhdr_next->bhdr_next;
    
    /*  We update the bhdr_prev pointer in the block on the right. Note that we
        must work on bhdr->bhdr_next and not bhdr->bhdr_next->bhdr_next now 
        because of the previous reassigment.    */
    /*  The branch is always taken due to dfree() always lowering the break 
        when the last block in list is freed.   */
    /*  LCOV_EXCL_      START. */
    if(bhdr->bhdr_next != NULL)
    {
        bhdr->bhdr_next->bhdr_prev = bhdr;
    }
    /*  LCOV_EXCL_      STOP. */

    return bhdr;
}



static inline blockheader *try_coalesce(blockheader *bhdr)
{
    while(bhdr->bhdr_next != NULL && bhdr->bhdr_next->is_free)
    {
        bhdr = do_coalesce_right(bhdr);
    }

    while(bhdr->bhdr_prev != NULL && bhdr->bhdr_prev->is_free)
    {
        bhdr = do_coalesce_right(bhdr->bhdr_prev);
    }

    return bhdr;
}



void dfree(void *p, void *arena)
{
    if(p == NULL)
    {
        return;
    }

    /*  LCOV_EXCL_      START. */
    if(arena == NULL)
    {
        arena = heap_start;
    }
    /*  LCOV_EXCL_      STOP. */

    blockheader *bhdr_p = GET_BLOCKHDR(p);
    arenaheader *ahdr = GET_ARENAHDR(arena);

// #ifdef DFREE_PEDANTIC
/*
    if((uintptr_t)p < (uintptr_t)ahdr->bhdr_first + AL_BLOCKHDR_SIZE
    || (uintptr_t)p > (uintptr_t)ahdr->arena_end - AL_BLOCKHDR_SIZE)
    {
        return;
    }

    blockheader *bhdr_curr = ahdr->bhdr_first;

    while(bhdr_curr != bhdr_p)
    {
        bhdr_curr = bhdr_curr->bhdr_next;
        if(bhdr_curr == NULL)
        {
            return;
        }
    }
*/
    if(bhdr_p->is_free)
    {
        return;
    }
// #endif

    bhdr_p->is_free = 1;

    bhdr_p = try_coalesce(bhdr_p);

    if(bhdr_p->bhdr_next == NULL)
    {
        if(bhdr_p->bhdr_prev != NULL)
        {
            bhdr_p->bhdr_prev->bhdr_next = NULL;
        }
        else
        {
            ahdr->bhdr_first = NULL;
        }

        ahdr->brkshifter(-(intptr_t)(AL_BLOCKHDR_SIZE + bhdr_p->payload_size),
                         ahdr);
    }     
}



void *dcalloc(size_t n, size_t size, void *arena)
{
    if(size != 0 && n > SIZE_MAX / size)
    {
        errno = ENOMEM;
        return NULL;
    }

    void *p = dmalloc(n * size, arena);

    if(p != NULL)
    {
        memset(p, 0, n * size);
    }

    return p;
}



void *drealloc(void *p, size_t size, void *dest, void *src)
{

    if(p == NULL)
    {
        return dmalloc(size, dest);
    }

    if(size == 0)
    {
        dfree(p, src);
        return p;
    }

    size = ALIGN(size);

    blockheader *bhdr_p = GET_BLOCKHDR(p);





    /*  1. dest and src differ. */
    if(dest != src)
    {
        void *p_new = dmalloc(size, dest);
        if(p_new == NULL)
        {
            errno = ENOMEM;
        }
        else
        {
            memcpy(p_new, p, MIN(size, bhdr_p->payload_size));
            dfree(p, src);
        }
        return p_new;
    }





    /*  2. Same size.   */
    if(size == bhdr_p->payload_size)
    {
        return p;
    }





    /*  3. To a smaller size.   */
    if(size < bhdr_p->payload_size)
    {
        if((bhdr_p->payload_size - size) >= MIN_BLOCK_SIZE)
        {
            do_split(bhdr_p, size);

            try_coalesce(bhdr_p->bhdr_next);

            /*  There's a chance we created a free block at the list end.   */
            if(bhdr_p->bhdr_next->bhdr_next == NULL)
            {
                /*  Allows us to use dfree to lower the break. If we don't
                    manually mark the block as free, dfree() will treat the
                    call as a double free case.  */
                bhdr_p->bhdr_next->is_free = 0;

                dfree((void *)
                    ((uintptr_t)bhdr_p->bhdr_next + AL_BLOCKHDR_SIZE), src);
            }
        }

        return p;
    }





    /*  4. To a bigger size mid-list with a free block next.    */
    if(bhdr_p->bhdr_next != NULL && bhdr_p->bhdr_next->is_free == 1
    && bhdr_p->payload_size
            + AL_BLOCKHDR_SIZE 
            + bhdr_p->bhdr_next->payload_size 
        >= size)
    {
        /*  We use right coalescing to merge with the next block before
            allocation and possible splitting.  */
        bhdr_p = do_coalesce_right(bhdr_p);
        try_split(bhdr_p, size);

        return p;
    }
    




    /*  5. To a bigger size at the end of the list. */
    if(bhdr_p->bhdr_next == NULL)
    {
        arenaheader *ahdr = GET_ARENAHDR(src);

        if(ahdr->brkshifter((intptr_t)(size - bhdr_p->payload_size), ahdr) 
                            != (void *)(-1))
        {
            bhdr_p->payload_size = size;
            return p;
        }

        /*
            If brkshifter() failed, we give it another try with the fallback
            allocation.    
        */
    }




    /*  6. Fallback allocation case.    */

    void *p_new = dmalloc(size, dest);

    if(p_new == NULL)
    {
        errno = ENOMEM;
        return NULL;
    }

    memcpy(p_new, p, bhdr_p->payload_size);
    dfree(p, src);
    return p_new;
}



void *dreallocarray(void *p, size_t n, size_t size, void *dest, void *src)
{
    if(size != 0 && n > SIZE_MAX / size)
    {
        errno = ENOMEM;
        return NULL;
    }

    return drealloc(p, n * size, dest, src);
}
