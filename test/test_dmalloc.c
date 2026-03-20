/*
    dmalloc memory allocator

    Check the project repo at https://github.com/sizeof-dario/dmalloc.git for
    more information about it.

******************************************************************************/

#include "test_dmalloc.h"

#define ARENA_SIZE 512

static char arena[ARENA_SIZE];

void setUp()
{
    if(arenainit(arena, ARENA_SIZE) < 0)
    {
        perror("arenainit");
        exit(EXIT_FAILURE);
    }
}

void tearDown()
{

}

void test_arenainit()
{
    arenaheader *ahdr = GET_ARENAHDR((uintptr_t)arena);

    TEST_ASSERT_BITS_LOW(alignof(max_align_t) - 1, ahdr);

    TEST_ASSERT_EQUAL_PTR((uintptr_t)ahdr + AL_ARENAHDR_SIZE, ahdr->arena_start);
    TEST_ASSERT_EQUAL_PTR((uintptr_t)ahdr + AL_ARENAHDR_SIZE, ahdr->arena_brk);
    TEST_ASSERT_EQUAL_PTR((uintptr_t)arena + ARENA_SIZE, ahdr->arena_end);        
    TEST_ASSERT_NULL(ahdr->bhdr_first);
    TEST_ASSERT_EQUAL_PTR(arenasbrk, ahdr->brkshifter);
}

void test_dmalloc_single_allocation()
{
    int values[] = {79814, -9280134, 16, -1203987};

    int *p = (int *)dmalloc(4 * sizeof(int), arena);

    TEST_ASSERT_NOT_NULL(p);

    for (int i = 0; i < 4; i++)
    {
        p[i] = values[i];
    }
    
    blockheader *bhdr = GET_BLOCKHDR(p);

    TEST_ASSERT_EQUAL_size_t(ALIGN(4 * sizeof(int)), bhdr->payload_size);
    TEST_ASSERT_FALSE(bhdr->is_free);
    TEST_ASSERT_NULL(bhdr->bhdr_prev);
    TEST_ASSERT_NULL(bhdr->bhdr_next);

    for (int j = 0; j < 4; j++)
    {
        TEST_ASSERT_EQUAL_INT(values[j], 
            *((int *)((uintptr_t)bhdr + AL_BLOCKHDR_SIZE) + j));
    }
    
    dfree(p, arena);
}

void test_dmalloc_multiple_allocation()
{
    char   *p = (char *)dmalloc(12 * sizeof(char), arena);
    short  *q = (short *)dmalloc(7 * sizeof(short), arena);
    int    *r = (int *)dmalloc(4 * sizeof(int), arena);
    double *s = (double *)dmalloc(3 * sizeof(double), arena);

    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_NOT_NULL(q);
    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_NOT_NULL(s);

    blockheader *bhdr_p = GET_BLOCKHDR(p);
    blockheader *bhdr_q = GET_BLOCKHDR(q);
    blockheader *bhdr_r = GET_BLOCKHDR(r);
    blockheader *bhdr_s = GET_BLOCKHDR(s);

    TEST_ASSERT_EQUAL_size_t(ALIGN(12 * sizeof(char)), bhdr_p->payload_size);
    TEST_ASSERT_FALSE(bhdr_p->is_free);
    TEST_ASSERT_NULL(bhdr_p->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_q, bhdr_p->bhdr_next);

    TEST_ASSERT_EQUAL_size_t(ALIGN(7 * sizeof(short)), bhdr_q->payload_size);
    TEST_ASSERT_FALSE(bhdr_q->is_free);
    TEST_ASSERT_EQUAL_PTR(bhdr_p, bhdr_q->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_r, bhdr_q->bhdr_next);

    TEST_ASSERT_EQUAL_size_t(ALIGN(4 * sizeof(int)), bhdr_r->payload_size);
    TEST_ASSERT_FALSE(bhdr_r->is_free);
    TEST_ASSERT_EQUAL_PTR(bhdr_q, bhdr_r->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_s, bhdr_r->bhdr_next);

    TEST_ASSERT_EQUAL_size_t(ALIGN(3 * sizeof(double)), bhdr_s->payload_size);
    TEST_ASSERT_FALSE(bhdr_s->is_free);
    TEST_ASSERT_EQUAL_PTR(bhdr_r, bhdr_s->bhdr_prev);
    TEST_ASSERT_NULL(bhdr_s->bhdr_next);

    dfree(p, arena);
    dfree(q, arena);
    dfree(r, arena);
    dfree(s, arena);
}

void test_dmalloc_free_blocks_use()
{
    double *p = (double *)dmalloc(4 * sizeof(double) + AL_BLOCKHDR_SIZE, arena);
    double *q = (double *)dmalloc(sizeof(double), arena);

    TEST_ASSERT_NOT_NULL(p);
    TEST_ASSERT_NOT_NULL(q);

    dfree(p, arena);

    double *r = (double *)dmalloc(2 * sizeof(double), arena);
    double *s = (double *)dmalloc(2 * sizeof(double), arena);

    TEST_ASSERT_NOT_NULL(r);
    TEST_ASSERT_NOT_NULL(s);

    blockheader *bhdr_r = GET_BLOCKHDR(r);
    blockheader *bhdr_s = GET_BLOCKHDR(s);
    blockheader *bhdr_q = GET_BLOCKHDR(q);


    /* Expected: r <-> s <-> q. */

    TEST_ASSERT_EQUAL_PTR(bhdr_s, bhdr_r->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_r, bhdr_s->bhdr_prev);
    TEST_ASSERT_EQUAL_PTR(bhdr_q, bhdr_s->bhdr_next);

    TEST_ASSERT_EQUAL_PTR(bhdr_s, bhdr_q->bhdr_prev);

    dfree(r, arena);
    dfree(s, arena);
    dfree(q, arena);
}

