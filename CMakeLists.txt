cmake_minimum_required(VERSION 3.28)

project(dmalloc C)

set(DMALLOC_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_alloc.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_arenas.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_libc_wrappers.c
)

add_library(dmalloc)

target_compile_options(dmalloc PRIVATE -Wall -Wextra -Werror -pedantic)

if(ENABLE_TESTING)
    target_compile_options(dmalloc PRIVATE --coverage -O0 -g)
    target_link_options(dmalloc PUBLIC --coverage)

    find_program(LCOV_PATH lcov)
    find_program(GENHTML_PATH genhtml)

    if(LCOV_PATH AND GENHTML_PATH)
        add_custom_target(coverage
            COMMAND ${LCOV_PATH} --directory . --capture --output-file coverage.info --rc branch_coverage=1
            COMMAND ${GENHTML_PATH} coverage.info --output-directory ./coverage_report --rc branch_coverage=1
            WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
    endif()
endif()

target_sources(dmalloc
    PRIVATE
        ${DMALLOC_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_alloc.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_arenas.h
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_defs.h

    PUBLIC
    FILE_SET HEADERS
    BASE_DIRS
      ${CMAKE_CURRENT_SOURCE_DIR}/include
    FILES
      ${CMAKE_CURRENT_SOURCE_DIR}/include/dmalloc.h
)

if(ENABLE_TESTING)
    enable_testing()
    add_executable(run_tests)

    target_sources(run_tests
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/test/unity/src/unity.c
            ${CMAKE_CURRENT_SOURCE_DIR}/test/test_dmalloc.c
    )
    
    target_include_directories(run_tests 
        PRIVATE
            ${CMAKE_CURRENT_SOURCE_DIR}/test/unity/src
            ${CMAKE_CURRENT_SOURCE_DIR}/src
    )

    target_link_libraries(run_tests dmalloc)

    add_test(NAME tests COMMAND run_tests)
endif()
