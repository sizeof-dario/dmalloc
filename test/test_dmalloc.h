/******************************************************************************
    
        dmalloc hybrid arena allocator.

    Project repo at https://github.com/sizeof-dario/dmalloc.git.

******************************************************************************/

/*  "test_dmalloc.h" - Master include file for the allocator unit tests.     */

#ifndef DMALLOC_UNIT_TESTS_H
#define DMALLOC_UNIT_TESTS_H 1

#include <stdlib.h>
#include "unity.h"
#include "dmalloc_internals.h"
#include "dmalloc.h"

void setUp();
void tearDown();

/*  Base tests. */

void test_arenasbrk_null_arena_header();
void test_arenasbrk_zero_delta();
void test_arenasbrk_overflow_protection();
void test_arenasbrk_out_of_bounds_protection();
void test_arenasbrk_successfull_shifts();



void test_arenainit_null_backing_memory();
void test_arenainit_not_enough_space();
void test_arenainit_successfull_initialization();



void test_dmalloc_successfull_allocations();
void test_dmalloc_unsuccessfull_allocation();
void test_dmalloc_free_blocks_reuse();



void test_dfree_null_pointer();
void test_dfree_block_in_the_middle_and_double_free();
void test_dfree_block_at_the_end();
void test_dfree_last_block();
void test_dfree_multiple_deallocations();



void test_dcalloc_overflow_protection();
void test_dcalloc_memory_initialization();
void test_dcalloc_zero_size();  /* A test for `dmalloc(0, arena)` as well. */
void test_dcalloc_unsuccessfull_allocation();



void test_drealloc_null_p();
void test_drealloc_zero_size();
void test_drealloc_different_src_and_dest();
void test_drealloc_same_size();
void test_drealloc_smaller_size();
/*  Two following test also cover for the fallback success/failure cases. */

void test_drealloc_bigger_size_in_the_middle();
void test_drealloc_bigger_size_at_the_end();
/*  ***********************************************************************/

void test_drealloc_reallocation_failure();



void test_dreallocarray_overflow_protection();
void test_dreallocarray_zero_size();
void test_dreallocarray_unsuccessfull_reallocation();

#endif /* DMALLOC_UNIT_TESTS_H */
