cmake_minimum_required(VERSION 3.28)

project(dmalloc C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Werror -pedantic")

option(ENABLE_COVERAGE "Enable coverage reporting" OFF)

add_library(dmalloc)

if(ENABLE_COVERAGE)
    message("ENABLE_COVERAGE is enabled! You can disable it with -DENABLE_COVERAGE=OFF.")

    # Add extra compiation flags to dmalloc.
    # --coverage: Enables converage tracking (.gcno files).
    #        -O0: Disables optimization.
    #         -g: Adds debug symbols used to link the coverage data to the code lines.
    target_compile_options(dmalloc PRIVATE --coverage -O0 -g)
    
    # Tells the linker to include the gcov library.
    target_link_options(dmalloc PUBLIC --coverage)


    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info --rc branch_coverage=1
            COMMAND ${GENHTML_PATH} coverage.info --output-directory ./coverage_report --rc branch_coverage=1
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif()
else()
    message("ENABLE_COVERAGE is disabled! You can enable it with -DENABLE_COVERAGE=ON.")
endif()



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
