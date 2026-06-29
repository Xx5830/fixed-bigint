cmake_minimum_required(VERSION 3.15)
project(fixed-bigint VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(FIXED_BIGINT_BUILD_TESTS "Build unit tests" ON)
option(FIXED_BIGINT_BUILD_EXAMPLES "Build examples" ON)

add_library(fixed_bigint_warnings INTERFACE)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(fixed_bigint_warnings INTERFACE -Wall -Wextra -Wpedantic -Wconversion -Wsign-conversion)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    target_compile_options(fixed_bigint_warnings INTERFACE /W4)
endif()

add_subdirectory(include)

if(FIXED_BIGINT_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

if(FIXED_BIGINT_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()