# Set the minimum required version of CMake
cmake_minimum_required(VERSION 3.14)

# Set the project name and C++ standard
project(JobSystem VERSION 1.0)
add_compile_definitions(_POSIX_C_SOURCE=200809L)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Testing ---
# Enable testing with CTest
enable_testing()

# Add the subdirectory containing the GoogleTest source
add_subdirectory(tests/lib/googletest)

# Add our own tests subdirectory. This is where we'll define our test executable.
add_subdirectory(tests)