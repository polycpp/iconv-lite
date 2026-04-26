cmake_minimum_required(VERSION 3.20)
project(polycpp_iconv_lite VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

set(POLYCPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(POLYCPP_SOURCE_DIR "" CACHE PATH "Path to a local polycpp checkout")
set(POLYCPP_ICONV_LITE_POLYCPP_UNICODE "builtin" CACHE STRING "Default polycpp Unicode backend for this companion when POLYCPP_UNICODE is not already set")
set_property(CACHE POLYCPP_ICONV_LITE_POLYCPP_UNICODE PROPERTY STRINGS auto icu builtin)

if(NOT DEFINED POLYCPP_UNICODE)
    set(POLYCPP_UNICODE "${POLYCPP_ICONV_LITE_POLYCPP_UNICODE}" CACHE STRING "Unicode support for embedded polycpp")
    set_property(CACHE POLYCPP_UNICODE PROPERTY STRINGS auto icu builtin)
endif()


if(POLYCPP_SOURCE_DIR AND EXISTS "${POLYCPP_SOURCE_DIR}/CMakeLists.txt")
    FetchContent_Declare(polycpp SOURCE_DIR "${POLYCPP_SOURCE_DIR}")
else()
    FetchContent_Declare(
        polycpp
        GIT_REPOSITORY git@github.com:enricohuang/polycpp.git
        GIT_TAG        master
        GIT_SHALLOW    TRUE
    )
endif()
FetchContent_MakeAvailable(polycpp)

add_library(polycpp_iconv_lite
    src/iconv_lite.cpp
)
target_link_libraries(polycpp_iconv_lite PUBLIC polycpp)
target_include_directories(polycpp_iconv_lite PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
target_compile_features(polycpp_iconv_lite PUBLIC cxx_std_20)

add_library(polycpp::iconv_lite ALIAS polycpp_iconv_lite)

set(POLYCPP_ICONV_LITE_BUILD_TESTS_DEFAULT OFF)
if(CMAKE_SOURCE_DIR STREQUAL PROJECT_SOURCE_DIR)
    set(POLYCPP_ICONV_LITE_BUILD_TESTS_DEFAULT ON)
endif()
option(POLYCPP_ICONV_LITE_BUILD_TESTS "Build tests" ${POLYCPP_ICONV_LITE_BUILD_TESTS_DEFAULT})
if(POLYCPP_ICONV_LITE_BUILD_TESTS)
    enable_testing()
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.15.2
    )
    FetchContent_MakeAvailable(googletest)

    set(POLYCPP_ICONV_LITE_UPSTREAM_TEST_SOURCES
        tests/upstream/big5_test.cpp
        tests/upstream/bom_test.cpp
        tests/upstream/cesu8_test.cpp
        tests/upstream/cyrillic_test.cpp
        tests/upstream/gbk_test.cpp
        tests/upstream/greek_test.cpp
        tests/upstream/main_test.cpp
        tests/upstream/shiftjis_test.cpp
        tests/upstream/streams_test.cpp
        tests/upstream/turkish_test.cpp
        tests/upstream/types_test.cpp
        tests/upstream/utf16_test.cpp
        tests/upstream/utf32_test.cpp
        tests/upstream/utf7_test.cpp
    )

    add_executable(polycpp_iconv_lite_test_smoke tests/test_smoke.cpp)
    target_link_libraries(polycpp_iconv_lite_test_smoke PRIVATE polycpp::iconv_lite GTest::gtest_main)

    add_executable(polycpp_iconv_lite_test_upstream_compat ${POLYCPP_ICONV_LITE_UPSTREAM_TEST_SOURCES})
    target_link_libraries(polycpp_iconv_lite_test_upstream_compat PRIVATE polycpp::iconv_lite GTest::gtest_main)

    include(GoogleTest)
    gtest_discover_tests(polycpp_iconv_lite_test_smoke)
    gtest_discover_tests(polycpp_iconv_lite_test_upstream_compat)
endif()

option(POLYCPP_ICONV_LITE_BUILD_EXAMPLES "Build examples" OFF)
if(POLYCPP_ICONV_LITE_BUILD_EXAMPLES)
    file(GLOB POLYCPP_ICONV_LITE_EXAMPLES CONFIGURE_DEPENDS examples/*.cpp)
    foreach(example_source IN LISTS POLYCPP_ICONV_LITE_EXAMPLES)
        get_filename_component(example_name "${example_source}" NAME_WE)
        set(example_target "polycpp_iconv_lite_example_${example_name}")
        add_executable("${example_target}" "${example_source}")
        target_link_libraries("${example_target}" PRIVATE polycpp::iconv_lite)
        set_target_properties("${example_target}" PROPERTIES
            OUTPUT_NAME "${example_name}"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/examples"
        )
    endforeach()
endif()
