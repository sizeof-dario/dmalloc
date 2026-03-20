/*
    dmalloc memory allocator

    Check the project repo at https://github.com/sizeof-dario/dmalloc.git for
    more information about it.

******************************************************************************/

#include "dmalloc_internals.h"
#include "dmalloc.h"

static void *heap_start = NULL;



void *arenasbrk(intptr_t increment, arenaheader *ahdr)
{
    if(ahdr == NULL)
    {
        errno = EINVAL;
        return (void *)(-1);
    }

    if(increment == 0)
    {
        return ahdr->arena_brk;
    }

    intptr_t shifted_brk = (intptr_t)ahdr->arena_brk + increment;

    if((increment < 0 && shifted_brk >= (intptr_t)ahdr->arena_brk)
    || (increment > 0 && shifted_brk <= (intptr_t)ahdr->arena_brk))
    {
        errno = EOVERFLOW;
        return (void *)(-1);
    }

    if(shifted_brk < (intptr_t)ahdr->arena_start
    || shifted_brk > (intptr_t)ahdr->arena_end)
    {
        errno = ERANGE;
        return (void *)(-1);
    }

    void *old_brk = ahdr->arena_brk;
    ahdr->arena_brk = (void *)shifted_brk;

    return old_brk;
}



static inline void* sbrk_wrap(intptr_t increment, arenaheader *dummy)
{
    UNUSED(dummy);
    return sbrk(increment);
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
    if(bhdr_next_old != NULL)
    {
        bhdr_next_old->bhdr_prev = bhdr_new;
    }
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



int arenainit(void *buffer, size_t size)
{
    if(buffer == NULL || size == 0)
    {
        errno = EINVAL;
        return -1;
    }

    arenaheader *hdr = GET_ARENAHDR((uintptr_t)buffer);

    ptrdiff_t padding = (uintptr_t)hdr - (uintptr_t)buffer;

    if(padding + AL_ARENAHDR_SIZE > size)
    {
        errno = ENOMEM;
        return -1;
    }

    size_t capacity = size - padding - AL_ARENAHDR_SIZE;

    hdr->arena_start    = (void *)((uintptr_t)hdr + AL_ARENAHDR_SIZE);
    hdr->arena_brk      = hdr->arena_start;
    hdr->arena_end      = (void *)((uintptr_t)hdr->arena_start + capacity);
    hdr->bhdr_first     = NULL;
    hdr->brkshifter     = arenasbrk;

    return 0;
}



int heapinit()
{
    arenaheader *hdr = GET_ARENAHDR((uintptr_t)heap_start);
    ptrdiff_t padding = (uintptr_t)hdr - (uintptr_t)heap_start;

    if(sbrk(padding + AL_ARENAHDR_SIZE) == (void *)(-1))
    {
        return -1;
    }

    hdr->arena_start    = (void *)((uintptr_t)hdr + AL_ARENAHDR_SIZE);
    hdr->arena_brk      = hdr->arena_start;
    hdr->arena_end      = (void *)((uintptr_t)hdr->arena_start + SIZE_MAX);
    hdr->bhdr_first     = NULL;
    hdr->brkshifter     = sbrk_wrap;

    return 0;
}



void *dmalloc(size_t size, void *arena)
{
    /*  Upon first call, we shall retrieve the heap starting address value and
        initalize the heap. */
    if(heap_start == NULL)
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

    arenaheader *ahdr = GET_ARENAHDR((uintptr_t)arena); 

    /*  We must guarantee memory alignment to ensure defined behaviour
        according to the C standard.    */
    size_t aligned_size = ALIGN(size);
    
    if(aligned_size < size || aligned_size > SIZE_MAX - AL_BLOCKHDR_SIZE)
    {
        errno = ENOMEM;
        return NULL;
    }

    size = aligned_size;





    /*  To allocate the memory, we first assume some allocated blocks already
        exist; thus, we traverse their list to try finding a block with a big
        enough payload size for reusage.    */

    blockheader *bhdr_start = ahdr->bhdr_first;
    blockheader *bhdr_curr  = bhdr_start;
    blockheader *bhdr_last  = NULL;                   

    /*  The first condition checks if there are no blocks in the heap;
        The second ones tells if we reached the end of the list.        */
    while(bhdr_start != NULL && bhdr_curr != NULL)
    {
        /*  We want to find a big enough block that is free.    */
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
        to begin with, we need to raise the break and we create a new block. */
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
    /*  We update the payload size making sure to also include the extra header
        space.  */
    bhdr->payload_size += (AL_BLOCKHDR_SIZE + bhdr->bhdr_next->payload_size);

    /*  We update the hdr_next pointer in the block we are performing right
        coalescing on.  */
    bhdr->bhdr_next = bhdr->bhdr_next->bhdr_next;
    
    /*  We update the bhdr_prev pointer in the block on the right. Note that we
        must work on bhdr->bhdr_next and not bhdr->bhdr_next->bhdr_next now 
        because of the previous reassigment.    */
    if(bhdr->bhdr_next != NULL)
    {
        bhdr->bhdr_next->bhdr_prev = bhdr;
    }
    
    /*  We return the header pointer passed to the function. It's not really
        necessary, but I thinks it renders these functions more uniform.    */
    return bhdr;
}



static inline blockheader *try_coalesce(blockheader *bhdr)
{
    /*  As long as right free neighbours are found, we perform right 
        coalescing. */
    while(bhdr->bhdr_next != NULL && bhdr->bhdr_next->is_free)
    {
        bhdr = do_coalesce_right(bhdr);
    }

    /*  As long as left free neighbours are found, we perform left 
        coalescing. */
    while(bhdr->bhdr_prev != NULL && bhdr->bhdr_prev->is_free)
    {
        bhdr = do_coalesce_right(bhdr->bhdr_prev);
    }

    /*  We return the header pointer passed to the function because left
        coalescing changes it and so it must be updated in the caller.  */
    return bhdr;
}



void dfree(void *payload, void *arena)
{
    /*  As stated in N1570 §7.22.3.3 for the free() function: "If [payload] is
        a null pointer, no action occurs". We'll follow the standard.   */
    if(payload == NULL)
    {
        return;
    }

    /*  payload could not be a pointer to memory allocated by the allocator 
        functions. The standard for free() says that "if the argument does not
        match a pointer earlier returned by a memory management function [...] 
        the behavior is undefined". However, for the sake of trying to prevent
        any corruption, we'll check if payload is a valid pointer by comparing 
        its alleged header to every possible valid header pointer. Finding no 
        match means the pointer is probabily invalid, and in that case we do 
        nothing.    */

    blockheader *bhdr_payload = GET_BLOCKHDR(payload);
    arenaheader *ahdr = GET_ARENAHDR((uintptr_t)arena);

    blockheader *bhdr_curr = ahdr->bhdr_first;

    while(bhdr_curr != bhdr_payload)
    {
        bhdr_curr = bhdr_curr->bhdr_next;
        if(bhdr_curr == NULL)
        {
            /*  If we get into this branch, it means we found no falid match,
                thus payload is invalid and we just silently return.    */
            return;
        }
    }

    /*  The standard also calls undefined behaviour when a double free is
        attempted. We can make easly turn this scenario deterministic by
        silently returnign in such cases.   */
    if(bhdr_payload->is_free)
    {
        return;
    }





    /*  After the previous checks, we can assume payload is valid, in the
        sense that bhdr_payload corresponds to a valid header of our list that 
        we can deallocate.  */

    /*  Let's preemtively free the block.   */
    bhdr_payload->is_free = 1;

    /*  We can try block coalescing.        */
    bhdr_payload = try_coalesce(bhdr_payload);


    /*  If hdr happens to point to the last block in the arena, we can lower 
        the program break.                      */
    if(bhdr_payload->bhdr_next == NULL)
    {
        /*  Unless we also happen to be freeing the first block in the list, we
            want to update the new-last block of our list.  */
        if(bhdr_payload->bhdr_prev != NULL)
        {
            bhdr_payload->bhdr_prev->bhdr_next = NULL;
        }
        else
        {
            ahdr->bhdr_first = NULL;
        }

        ahdr->brkshifter
            (-(intptr_t)(AL_BLOCKHDR_SIZE + bhdr_payload->payload_size), ahdr);
    }     
}



void *dcalloc(size_t n_el, size_t size_el, void *arena)
{
    /*  If an overflow would happen, we return a NULL pointer.  */
    if(size_el != 0 && n_el > SIZE_MAX / size_el)
    {
        return NULL;
    }

    void *p = dmalloc(n_el * size_el, arena);

    if(p != NULL)
    {
        /*  We initialize the memory to 0.  */
        memset(p, 0, n_el * size_el);
    }

    return p;
}



void *drealloc(void *payload, size_t size, void *dest, void *src)
{
    /*  First, we need to handle the two trivial argument cases.    */

    /*  1. According to N1570 §7.22.3.5, realloc(NULL, size) must behave like 
        malloc(size). We will follow this standard with drealloc() and 
        dmalloc().  */
    if(payload == NULL)
    {
        return dmalloc(size, dest);
    }

    /*  2. The ISO C standard doesn't explicitly define what realloc() (and so
        drealloc()) should do when size is 0. The UNIX manual states at 
        https://man7.org/linux/man-pages/man3/realloc.3p.html that "If [size] 
        is 0 [you can return] a pointer to the allocated space [and free] the 
        memory object pointed to by [payload]. We choose this option. */
    if(size == 0)
    {
        dfree(payload, src);
        return payload;
    }





    /*  Now, drealloc() can perform different operations when reallocating the
        block, based on the resizing request and its position in the block 
        list. We'll see them after setting the stage. */

    size_t al_size = ALIGN(size);

    /*  Aligning the payload new size may increase its value, so we must 
    prevent a possible overflow.    */
    if(al_size < size || al_size > SIZE_MAX - AL_BLOCKHDR_SIZE)
    {
        /*  For this situation, the ISO C standard imposes that "if memory for
            the new object cannot be allocated, the old object is not 
            deallocated and its value is unchanged" and asks to return a
            pointer (in this case, unchanged) to the old object.            */
        return payload;
    }

    size = al_size;

    blockheader *bhdr_payload = GET_BLOCKHDR(payload);





    /*  We can now divide the drealloc() action into 6 cases.   */

    /* 0. dest and src differ.  */
    if(dest != src)
    {
        void *p = dmalloc(size, dest);
        if(p == NULL)
        {
            return payload;
        }
        else
        {
            memcpy(p, payload, MIN(size, bhdr_payload->payload_size));
            dfree(payload, src);
            return p;
        } 
    }





    /*  1. No change.   */
    if(size == bhdr_payload->payload_size)
    {
        return payload;
    }





    /*  2. The block must be shrunk. */
    if(size < bhdr_payload->payload_size)
    {
        /*  We can try to use block splitting if enough space becomes
            available. We perform the check that should be handled by
            try_split() and then call do_split() because we want to implement
            additional logic. In particular, we are considering the cases where
            we are shrinking a block that has a free block on the right, or no 
            right blocks at all.    */
        if((bhdr_payload->payload_size - size) >= MIN_BLOCK_SIZE)
        {
            do_split(bhdr_payload, size);

            /*  We might have a free block on the right to coalesce with.   */
            try_coalesce(bhdr_payload->bhdr_next);

            /*  Also, there's a possibility we created a free block at the end 
                of the heap.    */
            if(bhdr_payload->bhdr_next->bhdr_next == NULL)
            {
                /*  This will make the heap shrink. */
                dfree((void *)((uintptr_t)bhdr_payload->bhdr_next 
                                + AL_BLOCKHDR_SIZE), src);
            }
        }

        return payload;
    }





    /*  3. The block must grow and it's in the middle of the list.  */
    if(bhdr_payload->bhdr_next != NULL && bhdr_payload->bhdr_next->is_free == 1
    && bhdr_payload->payload_size + AL_BLOCKHDR_SIZE 
        + bhdr_payload->bhdr_next->payload_size >= size)
    {
        /*  We use right coalescing to merge the next block, found to be free,
            to the current one, before "taking what we need" and trying 
            splitting.  */
        bhdr_payload = do_coalesce_right(bhdr_payload);
        
        try_split(bhdr_payload, size);

        return payload;
    }
    




    /*  4. The block must grow and it's at the end of the list. */
    if(bhdr_payload->bhdr_next == NULL)
    {

        arenaheader *ahdr = GET_ARENAHDR((uintptr_t)src);

        if(ahdr->brkshifter((intptr_t)(size - bhdr_payload->payload_size), 
                            ahdr) != (void *)(-1))
        {
            bhdr_payload->payload_size = size;
            return payload;
        }

        /*
            If brkshifter() failed, we give it another try with the fallback
            allocation.    
        */
    }




    /*  5. Fallback allocation case. The following code is executed when we are
        forced to copy all the data to a new (bigger) location. */

    void *payload_new = dmalloc(size, src);

    if(payload_new == NULL)
    {
        return payload;
    }

    memcpy(payload_new, payload, bhdr_payload->payload_size);

    dfree(payload, src);

    return payload_new;
}



void *dreallocarray(void *p, size_t n, size_t size, void *dest, void *src)
{
    /*  If an overflow would happen, we return a NULL pointer.  */
    if(size != 0 && n > SIZE_MAX / size)
    {
        return NULL;
    }

    return drealloc(p, n * size, dest, src);
}
