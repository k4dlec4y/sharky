add_executable(run_tests
    bitmap_test.cpp
)

target_link_libraries(run_tests
    PRIVATE 
    libbmpsharky
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(run_tests)
