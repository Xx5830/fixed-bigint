include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.12.1
)
FetchContent_MakeAvailable(googletest)

add_executable(fixed_bigint_tests test_fixed_bigint.cpp)

target_link_libraries(fixed_bigint_tests
    PRIVATE
    fbi::fixed_bigint
    gtest_main
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(fixed_bigint_tests PRIVATE -fsanitize=address,undefined -g -O0)
    target_link_options(fixed_bigint_tests PRIVATE -fsanitize=address,undefined)
endif()

include(GoogleTest)
gtest_discover_tests(fixed_bigint_tests)