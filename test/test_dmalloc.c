/*
    dmalloc memory allocator

    Check the project repo at https://github.com/sizeof-dario/dmalloc.git for
    more information about it.

******************************************************************************/

#include <stdlib.h>
#include "unity.h"
#include "dmalloc_alloc.h"
#include "dmalloc_arenas.h"
#include "dmalloc_defs.h"
#include "dmalloc.h"

#define ANY_LONG 1825
#define ANY_N 12
#define ANY_INT ANY_N
#define ANY_VALID_POSITIVE_SHIFT 64
#define ANY_VALID_NEGATIVE_SHIFT (- ANY_VALID_POSITIVE_SHIFT / 2)
#define ANY_SRC arena
#define ANY_DEST arena


#define CAPACITY 2048

#define ANY_CAPACITY CAPACITY

static char arena[CAPACITY];

void setUp()
{
    TEST_ASSERT_EQUAL_INT(0, darenainit(arena, CAPACITY));
}

void tearDown()
{
    TEST_ASSERT_EQUAL_INT(0, darenadestroy(arena));
}

/*  arenasbrk() tests. */

void test_arenasbrk_null_arena_header()
{
    TEST_ASSERT_NULL(arenasbrk(ANY_LONG, NULL));
}

void test_arenasbrk_zero_delta()
{
    arenaheader *ahdr = GET_ARENAHDR(arena);
    TEST_ASSERT_EQUAL_PTR(ahdr->arena_brk, arenasbrk(0, ahdr));
}

void test_arenasbrk_overflow_protection()
{
    /*  Overflow could happen only if the arena is located at an already 
        high/low enough address so that adding the delta would cause an 
        overflow. We can't control where the arena lives on the stack, but we
        can simulate what arenasbrk() would do faking the break address.     */

    arenaheader *ahdr = GET_ARENAHDR(arena);
    
    uintptr_t fake_brk = LONG_MAX;
    ahdr->arena_brk = (void *)fake_brk;

    errno = 0;
    TEST_ASSERT_EQUAL_PTR((void *)(-1), arenasbrk(LONG_MIN, ahdr));
    TEST_ASSERT_EQUAL_INT(ENOMEM, errno);

    fake_brk = (uintptr_t)LONG_MAX + 2;
    ahdr->arena_brk = (void *)fake_brk;

    errno = 0;
    TEST_ASSERT_EQUAL_PTR((void *)(-1), arenasbrk(LONG_MAX, ahdr));
    TEST_ASSERT_EQUAL_INT(ENOMEM, errno);
}

void test_arenasbrk_out_of_bounds_protection()
{
    arenaheader *ahdr = GET_ARENAHDR(arena);

    errno = 0;
    TEST_ASSERT_EQUAL_PTR((void *)(-1), arenasbrk(-CAPACITY, ahdr));
    TEST_ASSERT_EQUAL_INT(ENOMEM, errno);

    errno = 0;
    TEST_ASSERT_EQUAL_PTR((void *)(-1), arenasbrk(CAPACITY, ahdr));
    TEST_ASSERT_EQUAL_INT(ENOMEM, errno);
}

void test_arenasbrk_successfull_shifts()
{
    arenaheader *ahdr = GET_ARENAHDR(arena);
    void *old_brk = ahdr->arena_brk;

    TEST_ASSERT_EQUAL_PTR(old_brk, arenasbrk(ANY_VALID_POSITIVE_SHIFT, ahdr));

    TEST_ASSERT_EQUAL_PTR(
        (void *)((uintptr_t)old_brk + ANY_VALID_POSITIVE_SHIFT), 
        ahdr->arena_brk
    );

    old_brk = ahdr->arena_brk;

    TEST_ASSERT_EQUAL_PTR(old_brk, arenasbrk(ANY_VALID_NEGATIVE_SHIFT, ahdr));
    
    TEST_ASSERT_EQUAL_PTR(
        (void *)((uintptr_t)old_brk + ANY_VALID_NEGATIVE_SHIFT),
        ahdr->arena_brk
    );
}






/*  arenainit() tests. */

void test_darenainit_null_backing_memory()
{
    int old_errno = errno;
    TEST_ASSERT_EQUAL_INT(0, darenainit(NULL, ANY_CAPACITY));
    TEST_ASSERT_EQUAL_INT(old_errno, errno);
}

void test_darenainit_not_enough_space()
{

    char buffer[AL_ARENAHDR_SIZE - 1];

    errno = 0;
     
    TEST_ASSERT_EQUAL_INT(-1, darenainit(buffer, AL_ARENAHDR_SIZE - 1));
    TEST_ASSERT_EQUAL_INT(ENOMEM, errno);
}

void test_darenainit_successfull_initialization()
{
    arenaheader *ahdr = GET_ARENAHDR(arena);

    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, ahdr);

    TEST_ASSERT_EQUAL_PTR((void *)((uintptr_t)ahdr + AL_ARENAHDR_SIZE), 
                          ahdr->arena_start);
    TEST_ASSERT_EQUAL_PTR((void *)((uintptr_t)ahdr + AL_ARENAHDR_SIZE),
                          ahdr->arena_brk);
    TEST_ASSERT_EQUAL_PTR((void *)((uintptr_t)arena + CAPACITY),
                          ahdr->arena_end);        
    TEST_ASSERT_NULL(ahdr->bhdr_first);
    TEST_ASSERT_EQUAL_PTR(arenasbrk, ahdr->brkshifter);
}






/*  dmalloc() tests. */

void test_dmalloc_successfull_allocations()
{
    arenaheader *ahdr = GET_ARENAHDR(arena);
    void *old_brk = ahdr->arena_brk;

    char   *p1 =   (char *)dmalloc(sizeof(char),   arena);
    TEST_ASSERT_NOT_NULL(p1);

    TEST_ASSERT_EQUAL_PTR(
        (void *)((char *)old_brk + AL_BLOCKHDR_SIZE + ALIGN(sizeof(char))),
        arenasbrk(0, ahdr));

    old_brk = arenasbrk(0, ahdr);

    int    *p2 =    (int *)dmalloc(sizeof(int),    arena);
    TEST_ASSERT_NOT_NULL(p2);

    TEST_ASSERT_EQUAL_PTR(
        (void *)((char *)old_brk + AL_BLOCKHDR_SIZE + ALIGN(sizeof(int))),
        arenasbrk(0, ahdr));

    old_brk = arenasbrk(0, ahdr);


    short  *p3 =  (short *)dmalloc(sizeof(short),  arena);
    TEST_ASSERT_NOT_NULL(p3);

    TEST_ASSERT_EQUAL_PTR(
        (void *)((char *)old_brk + AL_BLOCKHDR_SIZE + ALIGN(sizeof(short))),
        arenasbrk(0, ahdr));

    old_brk = arenasbrk(0, ahdr);

    double *p4 = (double *)dmalloc(sizeof(double), arena);
    TEST_ASSERT_NOT_NULL(p4);

    TEST_ASSERT_EQUAL_PTR(
        (void *)((char *)old_brk + AL_BLOCKHDR_SIZE + ALIGN(sizeof(double))),
        arenasbrk(0, ahdr));

    blockheader *bhdr_p1 = GET_BLOCKHDR(p1);
    blockheader *bhdr_p2 = GET_BLOCKHDR(p2);
    blockheader *bhdr_p3 = GET_BLOCKHDR(p3);
    blockheader *bhdr_p4 = GET_BLOCKHDR(p4);


    TEST_ASSERT_EQUAL_PTR(bhdr_p1, ahdr->bhdr_first);


    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, bhdr_p1);
    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, p1);


    TEST_ASSERT_EQUAL_size_t(ALIGN(sizeof(char)), bhdr_p1->payload_size);
    TEST_ASSERT_FALSE(bhdr_p1->is_free);
    TEST_ASSERT_NULL(bhdr_p1->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p2->bhdr_next);


    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, bhdr_p2);
    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, p2);


    TEST_ASSERT_EQUAL_size_t(ALIGN(sizeof(int)), bhdr_p2->payload_size);
    TEST_ASSERT_FALSE(bhdr_p2->is_free);
    TEST_ASSERT_EQUAL_PTR(bhdr_p1, bhdr_p2->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p2->bhdr_next);


    
    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, bhdr_p3);
    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, p3);


    TEST_ASSERT_EQUAL_size_t(ALIGN(sizeof(short)), bhdr_p3->payload_size);
    TEST_ASSERT_FALSE(bhdr_p3->is_free);
    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p3->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p4, bhdr_p3->bhdr_next);


    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, bhdr_p4);
    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, p4);


    TEST_ASSERT_EQUAL_size_t(ALIGN(sizeof(double)), bhdr_p4->payload_size);
    TEST_ASSERT_FALSE(bhdr_p4->is_free);
    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p4->bhdr_prev);
    TEST_ASSERT_NULL(bhdr_p4->bhdr_next);
}

void test_dmalloc_unsuccessfull_allocation()
{
    errno = 0;
    TEST_ASSERT_NULL(dmalloc(CAPACITY, arena));
    TEST_ASSERT_EQUAL_INT(ENOMEM, errno);
}

void test_dmalloc_free_blocks_reuse()
{
    /*  Case 1: a free block exists but it's too small. */

    char   *p1 =   (char *)dmalloc(sizeof(char),        arena);
    double *p2 = (double *)dmalloc(10 * sizeof(double), arena);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);

    blockheader *bhdr_p2 = GET_BLOCKHDR(p2);

    dfree(p1, arena);

    double *p3 = (double *)dmalloc(10 * sizeof(double), arena);
    TEST_ASSERT_NOT_NULL(p3);
    blockheader *bhdr_p3 = GET_BLOCKHDR(p3);

    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p2->bhdr_next);

    /*  Case 2: a free block can be used but can't be split. */

    char   *p4 =   (char *)dmalloc(sizeof(char),        arena);
    TEST_ASSERT_EQUAL_PTR(p4, p1);

    /*  Case 3: a free block can be used and gets split. */

    dfree(p2, arena);
    double *p5 = (double *)dmalloc(sizeof(double),      arena);
    TEST_ASSERT_EQUAL_PTR(p5, p2);
    blockheader *bhdr_p5 = GET_BLOCKHDR(p5);
    TEST_ASSERT_EQUAL_PTR(bhdr_p3->bhdr_prev, bhdr_p5->bhdr_next);
}





/*  dfree()* tests. */

void test_dfree_null_pointer()
{
    char old_arena[CAPACITY];
    memcpy(old_arena, arena, CAPACITY);

    dfree(NULL, arena);

    TEST_ASSERT_EQUAL_CHAR_ARRAY(old_arena, arena, CAPACITY);
}

void test_dfree_block_in_the_middle_and_double_free()
{
    char *p1 = (char *)dmalloc(sizeof(char), arena);
    char *p2 = (char *)dmalloc(sizeof(char), arena);

    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);

    blockheader *bhdr_p1 = GET_BLOCKHDR(p1);
    blockheader *bhdr_p2 = GET_BLOCKHDR(p2);

    dfree(p1, arena);

    TEST_ASSERT_EQUAL_size_t(ALIGN(sizeof(char)), bhdr_p1->payload_size);
    TEST_ASSERT_TRUE(bhdr_p1->is_free);
    TEST_ASSERT_NULL(bhdr_p1->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p1->bhdr_next);

    char old_arena[CAPACITY];
    memcpy(old_arena, arena, CAPACITY);

    dfree(p1, arena);

    TEST_ASSERT_EQUAL_CHAR_ARRAY(old_arena, arena, CAPACITY);
}

void test_dfree_block_at_the_end()
{
    char *p1 = (char *)dmalloc(sizeof(char), arena);

    void *old_brk = arenasbrk(0, GET_ARENAHDR(arena));

    char *p2 = (char *)dmalloc(sizeof(char), arena);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);

    blockheader *bhdr_p1 = GET_BLOCKHDR(p1);
    dfree(p2, arena);

    TEST_ASSERT_EQUAL_PTR(old_brk, arenasbrk(0, GET_ARENAHDR(arena)));

    TEST_ASSERT_NULL(bhdr_p1->bhdr_next);
}

void test_dfree_last_block()
{
    char *p = (char *)dmalloc(sizeof(char), arena);
    TEST_ASSERT_NOT_NULL(p);
    dfree(p, arena);

    arenaheader *ahdr = GET_ARENAHDR(arena);

    TEST_ASSERT_NULL(ahdr->bhdr_first);
}

void test_dfree_multiple_deallocations()
{
    char *p1 = (char *)dmalloc(sizeof(char), arena);
    char *p2 = (char *)dmalloc(sizeof(char), arena);
    char *p3 = (char *)dmalloc(sizeof(char), arena);
    char *p4 = (char *)dmalloc(sizeof(char), arena);
    char *p5 = (char *)dmalloc(sizeof(char), arena);

    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_NOT_NULL(p3);
    TEST_ASSERT_NOT_NULL(p4);
    TEST_ASSERT_NOT_NULL(p5);

    blockheader *bhdr_p1 = GET_BLOCKHDR(p1);
    blockheader *bhdr_p2 = GET_BLOCKHDR(p2);
    blockheader *bhdr_p3 = GET_BLOCKHDR(p3);
    blockheader *bhdr_p4 = GET_BLOCKHDR(p4);
    blockheader *bhdr_p5 = GET_BLOCKHDR(p5);

    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p1->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p1, bhdr_p2->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p2->bhdr_next);
    
    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p3->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p4, bhdr_p3->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p4->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p5, bhdr_p4->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p4, bhdr_p5->bhdr_prev);

    dfree(p3, arena);

    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p1->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p1, bhdr_p2->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p2->bhdr_next);
    
    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p3->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p4, bhdr_p3->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p4->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p5, bhdr_p4->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p4, bhdr_p5->bhdr_prev);

    dfree(p4, arena);

    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p1->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p1, bhdr_p2->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p2->bhdr_next);
    
    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p3->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p5, bhdr_p3->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p3, bhdr_p5->bhdr_prev);
    
    dfree(p2, arena);

    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p1->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p1, bhdr_p2->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_p5, bhdr_p2->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_p2, bhdr_p5->bhdr_prev);
}





/*  dcalloc() tests. */

void test_dcalloc_overflow_protection()
{
    void *p = dcalloc(SIZE_MAX, SIZE_MAX, arena);
    TEST_ASSERT_NULL(p);
}

void test_dcalloc_memory_initialization()
{
    int *p = (int *)dcalloc(ANY_N, sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p);

    TEST_ASSERT_EACH_EQUAL_INT(0, p, ANY_N);
}

void test_dcalloc_zero_size()
{
    void *p1 = dcalloc(ANY_N, 0, arena);
    int  *p2 = (int *)dmalloc(sizeof(int), arena);

    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);

    void *bhdr_p2 = (void *)GET_BLOCKHDR(p2);
    TEST_ASSERT_EQUAL_PTR(bhdr_p2, p1);
}

void test_dcalloc_unsuccessfull_allocation()
{
    void *p = dcalloc(ANY_N, CAPACITY, arena);
    TEST_ASSERT_NULL(p);
}





/*  drealloc() tests. */

void test_drealloc_null_p()
{
    int *p = (int *)drealloc(NULL, sizeof(int), arena, ANY_SRC);
    TEST_ASSERT_NOT_NULL(p);

    arenaheader *ahdr = GET_ARENAHDR(arena);
    blockheader *bhdr_p = GET_BLOCKHDR(p);

    TEST_ASSERT_EQUAL_PTR(bhdr_p, ahdr->bhdr_first);
    TEST_ASSERT_EQUAL_size_t(ALIGN(sizeof(int)), bhdr_p->payload_size);
    TEST_ASSERT_FALSE(bhdr_p->is_free);
    TEST_ASSERT_NULL(bhdr_p->bhdr_prev);
    TEST_ASSERT_NULL(bhdr_p->bhdr_next);
}

void test_drealloc_zero_size()
{
    double *p1 = (double *)dmalloc(sizeof(double), arena);
    float  *p2 = (float *)dmalloc(sizeof(float), arena);

    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);

    blockheader *bhdr_p1 = GET_BLOCKHDR(p1);

    void *p3 = drealloc(p1, 0, ANY_DEST, arena);

    TEST_ASSERT_EQUAL_PTR(p1, p3);
    TEST_ASSERT_TRUE(bhdr_p1->is_free);
}

void test_drealloc_different_src_and_dest()
{
    char other_arena[CAPACITY];

    TEST_ASSERT_EQUAL_INT(0, darenainit(other_arena, CAPACITY));

    int *p1 = (int *)dmalloc(sizeof(int), arena);
    int *p2 = (int *)dmalloc(sizeof(int), arena);

    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);

    *p1 = ANY_INT;

    blockheader *bhdr_p1 = GET_BLOCKHDR(p1);

    int *p3 = (int *)drealloc(p1, sizeof(int), other_arena, arena);

    TEST_ASSERT_NOT_NULL(p3);

    blockheader *bhdr_p3 = GET_BLOCKHDR(p3);

    TEST_ASSERT_TRUE(bhdr_p1->is_free);

    TEST_ASSERT_EQUAL_INT(ANY_INT, *p3);

    errno = 0;
    int *p4 = (int *)drealloc(p3, CAPACITY, arena, other_arena);
    TEST_ASSERT_NULL(p4);
    TEST_ASSERT_EQUAL_INT(ENOMEM, errno);

    TEST_ASSERT_FALSE(bhdr_p3->is_free);
}

void test_drealloc_same_size()
{
    int *p1 = (int *)dmalloc(sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p1);

    int *p2 = (int *)drealloc(p1 ,sizeof(int), arena, arena);
    TEST_ASSERT_EQUAL_PTR(p1, p2);
}

void test_drealloc_smaller_size()
{
    arenaheader *ahdr = GET_ARENAHDR(arena);

    /*  1. No new block are spawn. */
    double *p1 = (double *)dmalloc(26*sizeof(double), arena);
    TEST_ASSERT_NOT_NULL(p1);

    void *old_brk = arenasbrk(0, ahdr);

    double *p2 = (double *)drealloc(p1, 21*sizeof(double), arena, arena);
    TEST_ASSERT_EQUAL_PTR(p1 , p2);
    TEST_ASSERT_EQUAL_PTR(old_brk, arenasbrk(0, ahdr));
    old_brk = arenasbrk(0, ahdr);

    /*  2. New block at the end of the list. */
    
    double *p3 = (double *)drealloc(p2, 11*sizeof(double), arena, arena);
    TEST_ASSERT_EQUAL_PTR(p1 , p3);
    TEST_ASSERT_GREATER_THAN_UINT(
        0, (uintptr_t)old_brk - (uintptr_t)arenasbrk(0, ahdr));
    old_brk = arenasbrk(0, ahdr);

    /*  3. New block in the middle of the list. */
    char *p4 = (char *)dmalloc(sizeof(char), arena);
    TEST_ASSERT_NOT_NULL(p4);
    old_brk = arenasbrk(0, ahdr);

    double *p5 = (double *)drealloc(p3, sizeof(double), arena, arena);
    TEST_ASSERT_EQUAL_PTR(p1 , p5);
    TEST_ASSERT_EQUAL_PTR(old_brk, arenasbrk(0, ahdr));
}

void test_drealloc_bigger_size_in_the_middle()
{
    int *p1 = (int *)dmalloc(sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p1);

    int *p2 = (int *)dmalloc(sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p2);
    blockheader *bhdr_p2 = GET_BLOCKHDR(p2);

    int *p3 = (int *)dmalloc(sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p3);
    blockheader *bhdr_p3 = GET_BLOCKHDR(p3);
    
    int *p4 = (int *)dmalloc(sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p4);
    blockheader *bhdr_p4 = GET_BLOCKHDR(p4);

    /*  Test logic: p2 is freed and it's enough. Then p3 is freed but it's not
         enough. */

    dfree(p2, arena);
    TEST_ASSERT_TRUE(bhdr_p2->is_free);

    int *q1 = (int *)drealloc(p1, 2*ALIGN(sizeof(int))+AL_BLOCKHDR_SIZE, 
                         arena, arena);
 
    /*  We expect: q1 <-> p3 <-> p4. */

    TEST_ASSERT_EQUAL(p1, q1);

    blockheader *bhdr_q1 = GET_BLOCKHDR(q1);
    TEST_ASSERT_EQUAL_PTR(bhdr_q1, bhdr_p2->bhdr_prev);

    /*  ******************************/

    dfree(p3, arena);
    TEST_ASSERT_TRUE(bhdr_p3->is_free);

    /*        Freed and merged to p3. */
    /*             v                  */
    /*  We expect (q1) <-> p4 <-> p5. */

    int *p5 = (int *)drealloc(q1, (3+1)*ALIGN(sizeof(int))+2*AL_BLOCKHDR_SIZE, 
                              arena, arena);

    TEST_ASSERT_NOT_NULL(p5);
    blockheader *bhdr_p5 = GET_BLOCKHDR(p5);

    TEST_ASSERT_EQUAL_PTR(bhdr_p5, bhdr_p4->bhdr_next);

    /*  We also check what happens if there's not a free block to begin with.*/

    int *p6 = (int *)drealloc(p4, 2*ALIGN(sizeof(int)), arena, arena);
    TEST_ASSERT_NOT_NULL(p4);

    /*  In this case, we expect p6 to become first in list. */
    blockheader *bhdr_p6 = GET_BLOCKHDR(p6);


    arenaheader *ahdr = GET_ARENAHDR(arena);
    TEST_ASSERT_EQUAL_PTR(bhdr_p6, ahdr->bhdr_first);
}

void test_drealloc_bigger_size_at_the_end()
{
    int *p1 = (int *)dmalloc(sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p1);
    blockheader *bhdr_p1 = GET_BLOCKHDR(p1);
    size_t old_size = bhdr_p1->payload_size;

    /*  Success. */

    int *p2 = (int *)drealloc(p1, 2*ALIGN(sizeof(int)), arena, arena);
    TEST_ASSERT_NOT_NULL(p2);

    TEST_ASSERT_EQUAL_PTR(p1, p2);
    TEST_ASSERT_GREATER_THAN(old_size, bhdr_p1->payload_size);

    /*  Failure. */

    int *p3 = (int *)drealloc(p2, CAPACITY, arena, arena);
    TEST_ASSERT_NULL(p3);
}

void test_drealloc_reallocation_failure()
{
    int *p1 = (int *)dmalloc(ANY_N*sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p1);
    blockheader *bhdr_p1 = GET_BLOCKHDR(p1);

    for (int i = 0; i < ANY_N; i++)
    {
        p1[i] = ANY_INT;
    }
    
    int *p2 = (int *)drealloc(p1, CAPACITY, arena , arena);
    TEST_ASSERT_NULL(p2);

    TEST_ASSERT_EQUAL_size_t(ALIGN(ANY_N*sizeof(int)), bhdr_p1->payload_size);
    TEST_ASSERT_FALSE(bhdr_p1->is_free);
    TEST_ASSERT_NULL(bhdr_p1->bhdr_prev);
    TEST_ASSERT_NULL(bhdr_p1->bhdr_next);

    for (int i = 0; i < ANY_N; i++)
    {
        TEST_ASSERT_EQUAL_INT(ANY_INT, p1[i]);
    }
}



/*  dreallocarray() tests. */

void test_dreallocarray_overflow_protection()
{
    void *p1 = dcalloc(ANY_N, sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p1);

    errno = 0;
    void *p2 = dreallocarray(p1, SIZE_MAX, SIZE_MAX, arena, arena);
    TEST_ASSERT_NULL(p2);
    TEST_ASSERT_EQUAL_INT(ENOMEM, errno);
}

void test_dreallocarray_zero_size()
{
    int *p1 = (int *)dcalloc(ANY_N, sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p1);

    int *p2 = dmalloc(1, arena);
    TEST_ASSERT_NOT_NULL(p2);

    int *p3 = (int *)dreallocarray(p1, ANY_N, 0, arena, arena);
    TEST_ASSERT_NOT_NULL(p3);
    blockheader *bhdr_p3 = GET_BLOCKHDR(p3);

    TEST_ASSERT_EQUAL_PTR(p1, p3);
    TEST_ASSERT_TRUE(bhdr_p3->is_free);
}

void test_dreallocarray_unsuccessfull_reallocation()
{
    void *p1 = dcalloc(ANY_N, sizeof(int), arena);
    TEST_ASSERT_NOT_NULL(p1);

    void *p2 = dreallocarray(p1, ANY_N, CAPACITY, arena, arena);
    TEST_ASSERT_NULL(p2);
}

/******************************************************************** */




int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_arenasbrk_null_arena_header);
    RUN_TEST(test_arenasbrk_zero_delta);
    RUN_TEST(test_arenasbrk_overflow_protection);
    RUN_TEST(test_arenasbrk_out_of_bounds_protection);
    RUN_TEST(test_arenasbrk_successfull_shifts);

    RUN_TEST(test_darenainit_null_backing_memory);
    RUN_TEST(test_darenainit_not_enough_space);
    RUN_TEST(test_darenainit_successfull_initialization);

    RUN_TEST(test_dmalloc_successfull_allocations);
    RUN_TEST(test_dmalloc_unsuccessfull_allocation);
    RUN_TEST(test_dmalloc_free_blocks_reuse);

    RUN_TEST(test_dfree_null_pointer);
    RUN_TEST(test_dfree_block_in_the_middle_and_double_free);
    RUN_TEST(test_dfree_block_at_the_end);
    RUN_TEST(test_dfree_last_block);
    RUN_TEST(test_dfree_multiple_deallocations);

    RUN_TEST(test_dcalloc_overflow_protection);
    RUN_TEST(test_dcalloc_memory_initialization);
    RUN_TEST(test_dcalloc_zero_size);
    RUN_TEST(test_dcalloc_unsuccessfull_allocation);

    RUN_TEST(test_drealloc_null_p);
    RUN_TEST(test_drealloc_zero_size);
    RUN_TEST(test_drealloc_different_src_and_dest);
    RUN_TEST(test_drealloc_same_size);
    RUN_TEST(test_drealloc_smaller_size);
    RUN_TEST(test_drealloc_bigger_size_in_the_middle);
    RUN_TEST(test_drealloc_bigger_size_at_the_end);
    RUN_TEST(test_drealloc_reallocation_failure);

    RUN_TEST(test_dreallocarray_overflow_protection);
    RUN_TEST(test_dreallocarray_zero_size);
    RUN_TEST(test_dreallocarray_unsuccessfull_reallocation);

    return UNITY_END();
}