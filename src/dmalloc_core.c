/*  dmalloc - A didactic memory allocator for UNIX systems.

Project repository at https://github.com/sizeof-dario/dmalloc.git.
Documentation is avaiable at https://sizeof-dario.github.io/dmalloc/. 

Latest change on date 2026-04-15. */

/**
 *  Implementation of allocator core functions "dmalloc_core.c".
 */


#include "dmalloc.h"
#include "dmalloc_core.h"
#include "dmalloc_arenas.h"

#ifdef DMALLOC_DEBUG
 #include "dmalloc_debug.h"
#endif

extern void *heap_start; /* Found in "dmalloc_arenas.c". */



void do_split(blockheader *bhdr, size_t bhdr_payload_size)
{
    /*  We need to save the current data to later update the headers of the 
        blocks adjacent to the one we're about to split and initialize the 
        header of the extra block that will be created by the splitting.    */

    size_t payload_size_old = bhdr->payload_size;
    blockheader *bhdr_next_old = bhdr->bhdr_next;

    /*  We create a new header. */

    blockheader *bhdr_new = (blockheader *)
        ((uintptr_t)bhdr + AL_BLOCKHDR_SIZE + bhdr_payload_size);

    /*  We update the splitted block header. */
    bhdr->payload_size = bhdr_payload_size;
    bhdr->bhdr_next = bhdr_new;

    /*  We initialize the new block header. */
    bhdr_new->payload_size
        = payload_size_old - bhdr_payload_size - AL_BLOCKHDR_SIZE;
    bhdr_new->is_free = 1;
    bhdr_new->bhdr_prev = bhdr;
    bhdr_new->bhdr_next = bhdr_next_old;

    /*  We update the header of the block that next to the new one. */
    /*  The branch is always taken due to dfree() always lowering the break
        when the last block in list is freed. */

    /* LCOV_EXCL_ START. */

    if(bhdr_next_old != NULL)
    {
        bhdr_next_old->bhdr_prev = bhdr_new;
    }

    /*  LCOV_EXCL_ STOP. */
}

void try_split(blockheader *bhdr, size_t bhdr_payload_size)
{
    if((bhdr->payload_size - bhdr_payload_size) >= MIN_BLOCK_SIZE)
    {
        do_split(bhdr, bhdr_payload_size);
    }
}

void *dmalloc_unlocked(size_t size, arenaheader *ahdr)
{
    size_t al_size = ALIGN(size);



    blockheader *bhdr_start = ahdr->bhdr_first;
    blockheader *bhdr_curr = bhdr_start;
    blockheader *bhdr_last = NULL;
    while(bhdr_start != NULL && bhdr_curr != NULL)
    {
        if(bhdr_curr->is_free == 1 && (bhdr_curr->payload_size >= al_size))
        {
            bhdr_curr->is_free = 0;
            try_split(bhdr_curr, al_size);
            return (void *)((uintptr_t)bhdr_curr + AL_BLOCKHDR_SIZE);
        }

        /*  We keep track of the last accessed header so that if we exit the
            loop without returning, meaning we reached the end of the list,
            we can create a new block and link its header to it. */
        bhdr_last = bhdr_curr;
        bhdr_curr = bhdr_curr->bhdr_next;
    }

    /*  If we weren't able to find a suitable block, or if there were no blocks
        to begin with, we need to raise the break and create a new block. */
    
        blockheader *p = ahdr->brkshifter(al_size + AL_BLOCKHDR_SIZE, ahdr);
        if(p == (void *)(-1))
        {
            errno = ENOMEM;
            return NULL;
        }

        if(ahdr->bhdr_first == NULL)
        {
            ahdr->bhdr_first = p;
        }



        p->is_free = 0;
        p->payload_size = al_size;
        p->bhdr_prev = bhdr_last;
        p->bhdr_next = NULL;
        
        if(bhdr_last != NULL)
        {
            p->bhdr_prev->bhdr_next = p;
        }
        
        return (void *)((uintptr_t)p + AL_BLOCKHDR_SIZE);
}

void *dmalloc(size_t size, darena_t *arena)
{
    if(arena == NULL)
    {
        if(heapinit() < 0)
        {
            return NULL;
        }

        arena = heap_start;
    }

    arenaheader *ahdr = GET_ARENAHDR(arena);

    pthread_mutex_lock(&ahdr->lock);
    void *p = dmalloc_unlocked(size, ahdr);
    pthread_mutex_unlock(&ahdr->lock);

    return p;
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

blockheader *try_coalesce(blockheader *bhdr)
{
    while(bhdr != NULL && bhdr->bhdr_next != NULL && bhdr->bhdr_next->is_free)
    {
        bhdr = do_coalesce_right(bhdr);
    }

    while(bhdr != NULL && bhdr->bhdr_prev != NULL && bhdr->bhdr_prev->is_free)
    {
        bhdr = do_coalesce_right(bhdr->bhdr_prev);
    }

    return bhdr;
}

void dfree_unlocked(void *p, arenaheader *ahdr)
{
    if(p == NULL)
    {
        return;
    }

    blockheader *bhdr_p = GET_BLOCKHDR(p);



    blockheader *bhdr_curr = ahdr->bhdr_first;

    while(bhdr_curr != bhdr_p)
    {
        bhdr_curr = bhdr_curr->bhdr_next;
        if(bhdr_curr == NULL)
        {
            /*  If we get into this branch, it means we found no valid match
                for hdr, thus payload_ptr is invalid and we just silently 
                return. */
            return;
        }
    }



    if(bhdr_p->is_free)
    {
        return;
    }



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

        ahdr->
        brkshifter(-(intptr_t)(AL_BLOCKHDR_SIZE + bhdr_p->payload_size), ahdr);
    }     
}

void dfree(void *p, darena_t *arena)
{
    #ifdef DMALLOC_DEBUG
    write(1, "dfree(p = ", 10);
    char buf[30];
    ultos((unsigned long)p, buf, 30, 16, 1);
    write(1, buf, 30);
  
    if(p != NULL)
    {
    write(1, " - BLOCK HEADER address = ", 26);
    ultos((unsigned long)GET_BLOCKHDR(p), buf, 30, 16, 1);
    write(1, buf, 30);
    }

    write(1, ")\n\n", 3); 

    arenadump((void *)arena);
    #endif
   
    if(p == NULL)
    {
        return;
    }

    if(arena == NULL)
    {
        if(heap_start == NULL)
        {
            return;
        }

        arena = heap_start;
    }

    arenaheader *ahdr = GET_ARENAHDR(arena);

    pthread_mutex_lock(&ahdr->lock);
    dfree_unlocked(p, ahdr);
    pthread_mutex_unlock(&ahdr->lock);
}



void *dcalloc(size_t n, size_t size, darena_t *arena)
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

void *drealloc(void *p, size_t size, darena_t *dest, darena_t *src)
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

    if(dest == NULL)
    {
        if(heapinit() < 0)
        {
            return NULL;
        }

        dest = heap_start;
    }

    if(src == NULL)
    {
        if(heapinit() < 0)
        {
            return NULL;
        }

        src = heap_start;
    }

    /*  1. dest and src differ. */
    if(dest != src)
    {
        arenaheader *dest_hdr = GET_ARENAHDR(dest);
        arenaheader *src_hdr  = GET_ARENAHDR(src);

        /*  Always lock the lower first to avoid deadlock. */
        if((uintptr_t)dest_hdr < (uintptr_t)src_hdr)
        {
            pthread_mutex_lock(&dest_hdr->lock);
            pthread_mutex_lock(&src_hdr->lock);
        }
        else
        {
            pthread_mutex_lock(&src_hdr->lock);
            pthread_mutex_lock(&dest_hdr->lock);
        }

        void *p_new = dmalloc_unlocked(size, dest_hdr);
        if(p_new == NULL)
        {
            errno = ENOMEM;
        }
        else
        {
            size_t min_size = size < bhdr_p->payload_size ? size : bhdr_p->payload_size;
            memcpy(p_new, p, min_size);
            dfree_unlocked(p, src_hdr);
        }

        pthread_mutex_unlock(&dest_hdr->lock);
        pthread_mutex_unlock(&src_hdr->lock);

        return p_new;
    }





    /*  2. Same size.   */
    if(size == bhdr_p->payload_size)
    {
        return p;
    }



    arenaheader *ahdr = GET_ARENAHDR(src);
    pthread_mutex_lock(&ahdr->lock);

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

                dfree_unlocked((void *)
                    ((uintptr_t)bhdr_p->bhdr_next + AL_BLOCKHDR_SIZE), ahdr);
            }
        }

        pthread_mutex_unlock(&ahdr->lock);        
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

        pthread_mutex_unlock(&ahdr->lock);        
        return p;
    }
    




    /*  5. To a bigger size at the end of the list. */
    if(bhdr_p->bhdr_next == NULL)
    {
        if(ahdr->brkshifter((intptr_t)(size - bhdr_p->payload_size), ahdr) 
                            != (void *)(-1))
        {
            bhdr_p->payload_size = size;
            pthread_mutex_unlock(&ahdr->lock);
            return p;
        }

        /*
            If brkshifter() failed, we give it another try with the fallback
            allocation.    
        */
    }




    /*  6. Fallback allocation case.    */

    void *p_new = dmalloc_unlocked(size, ahdr);

    if(p_new == NULL)
    {
        errno = ENOMEM;
        pthread_mutex_unlock(&ahdr->lock);
        return NULL;
    }

    memcpy(p_new, p, bhdr_p->payload_size);
    dfree_unlocked(p, ahdr);

    pthread_mutex_unlock(&ahdr->lock);        
    return p_new;
}

void *dreallocarray(void *p, size_t n, size_t size, darena_t *dest, 
                    darena_t *src)
{
    if(size != 0 && n > SIZE_MAX / size)
    {
        errno = ENOMEM;
        return NULL;
    }

    return drealloc(p, n * size, dest, src);
}
