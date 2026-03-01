cmake_minimum_required(VERSION 3.23)
cmake_policy(SET CMP0135 NEW)

project(sharky)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -pedantic -Wold-style-cast -Werror -Iinclude)

add_library(libbmpsharky SHARED
    src/bitmap.cpp
    src/extract.cpp
    src/hide.cpp
)

target_include_directories(libbmpsharky
    PUBLIC include
)

add_executable(sharky
    src/main.cpp
)

target_link_libraries(sharky
    PRIVATE libbmpsharky
)

include(FetchContent)
FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/73a63ea05dc8ca29ec1d2c1d66481dd0de1950f1.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(run_tests
    src/tests.cpp
)

target_link_libraries(run_tests
    PRIVATE libbmpsharky
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)
