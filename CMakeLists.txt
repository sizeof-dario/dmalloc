cmake_minimum_required(VERSION 3.28)

project(dmalloc C)

set(DMALLOC_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_arenas.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_core.c
    ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_libc_wrappers.c   
)

set(DMALLOC_HEADERS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/dmalloc_arenas.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/dmalloc_core.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/dmalloc_defs.h
)

if(DMALLOC_DEBUG)
    set(DMALLOC_SOURCES
        ${DMALLOC_SOURCES}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/dmalloc_debug.c
    )
    set(DMALLOC_HEADERS
        ${DMALLOC_HEADERS}
        ${CMAKE_CURRENT_SOURCE_DIR}/src/headers/dmalloc_debug.h
    )

    add_compile_definitions(DMALLOC_DEBUG)
endif()

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

    PRIVATE
    FILE_SET privateHeaders
    TYPE HEADERS 
    BASE_DIRS
        ${CMAKE_CURRENT_SOURCE_DIR}/src/headers
    FILES
        ${DMALLOC_HEADERS}

    PUBLIC
    FILE_SET publicHeaders
    TYPE HEADERS
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
            ${CMAKE_CURRENT_SOURCE_DIR}/src/headers
    )

    target_link_libraries(run_tests dmalloc)

    add_test(NAME tests COMMAND run_tests)
endif()
