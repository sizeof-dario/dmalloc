cmake_minimum_required(VERSION 3.28)

project(dmalloc C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic")

add_library(dmalloc)

target_sources(dmalloc
    PRIVATE src/dmalloc.c

    PRIVATE FILE_SET dmalloc_private_headers
        TYPE HEADERS
        BASE_DIRS src
        FILES src/dmalloc_internals.h

    PUBLIC FILE_SET dmalloc_public_headers
        TYPE HEADERS
        BASE_DIRS include
        FILES include/dmalloc.h
) 

target_include_directories(dmalloc
    PRIVATE src
    PUBLIC include
)

enable_testing()

add_executable(run_base_tests)

target_sources(run_base_tests
    PRIVATE
        test/base_tests.c
        test/test_dmalloc.c
        test/unity/src/unity.c
    
    PRIVATE FILE_SET dmalloc_private_headers
        TYPE HEADERS
        BASE_DIRS src
        FILES src/dmalloc_internals.h

    PUBLIC FILE_SET dmalloc_public_headers
        TYPE HEADERS
        BASE_DIRS include
        FILES include/dmalloc.h

    PRIVATE FILE_SET test_dmalloc_private_headers
        TYPE HEADERS
        BASE_DIRS test
        FILES test/test_dmalloc.h

    PRIVATE
        FILE_SET test_dmalloc_private_unity_headers
        TYPE HEADERS
        BASE_DIRS test/unity/src
        FILES test/unity/src/unity.h
)

target_include_directories(run_base_tests
    PRIVATE test
    PRIVATE test/unity/src
)

target_link_libraries(run_base_tests dmalloc)

add_test(NAME base_tests COMMAND run_base_tests)
