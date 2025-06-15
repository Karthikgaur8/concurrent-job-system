# Add the executable for our test suite
add_executable(run_tests test_thread_pool.cpp)

# Link our test executable against GoogleTest
target_link_libraries(run_tests
    PRIVATE
    GTest::gtest_main
)

# Tell our test executable where to find our library's header files
target_include_directories(run_tests
    PRIVATE
    ../include
)

# Add this test executable to the list of tests CTest will run
include(GoogleTest)
gtest_discover_tests(run_tests)