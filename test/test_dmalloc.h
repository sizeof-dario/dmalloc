/*
    dmalloc memory allocator

    Check the project repo at https://github.com/sizeof-dario/dmalloc.git for
    more information about it.

******************************************************************************/

/*  "test.h" - Master include file for the allocator unit tests.             */

#ifndef DMALLOC_UNIT_TESTS_H
#define DMALLOC_UNIT_TESTS_H 1

#include <stdlib.h>
#include "unity.h"
#include "dmalloc_internals.h"
#include "dmalloc.h"

void setUp();
void tearDown();

/*  Base tests. */

void test_arenainit();

void test_dmalloc_single_allocation();
void test_dmalloc_multiple_allocation();
void test_dmalloc_free_blocks_use();

#endif /* DMALLOC_UNIT_TESTS_H */