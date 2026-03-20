/*
    dmalloc memory allocator

    Check the project repo at https://github.com/sizeof-dario/dmalloc.git for
    more information about it.

******************************************************************************/

#include "test_dmalloc.h"

int main()
{
    UNITY_BEGIN();

    RUN_TEST(test_arenainit);
    RUN_TEST(test_dmalloc_single_allocation);
    RUN_TEST(test_dmalloc_multiple_allocation);
    RUN_TEST(test_dmalloc_free_blocks_use);

    return UNITY_END();
}