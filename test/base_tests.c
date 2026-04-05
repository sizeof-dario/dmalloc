/*
    dmalloc memory allocator

    Check the project repo at https://github.com/sizeof-dario/dmalloc.git for
    more information about it.

******************************************************************************/

#include "test_dmalloc.h"

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_arenasbrk_null_arena_header);
    RUN_TEST(test_arenasbrk_zero_delta);
    RUN_TEST(test_arenasbrk_overflow_protection);
    RUN_TEST(test_arenasbrk_out_of_bounds_protection);
    RUN_TEST(test_arenasbrk_successfull_shifts);

    RUN_TEST(test_arenainit_null_backing_memory);
    RUN_TEST(test_arenainit_not_enough_space);
    RUN_TEST(test_arenainit_successfull_initialization);

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