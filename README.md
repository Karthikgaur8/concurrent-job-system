# Concurrent Job System

[![Language](https://img.shields.io/badge/Language-C%2B%2B17-blue.svg)](https://isocpp.org/)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)

A high-performance, header-only concurrent job system in modern C++ designed to parallelize tasks efficiently across multiple CPU cores. This library provides a powerful thread pool implementation that supports future-based results and complex task dependencies.

This project was built from scratch to demonstrate advanced C++ concurrency, API design, and modern software engineering practices. It expands on concepts from my data structures and algorithms coursework at the University of Alabama, focusing on performance optimization.

## Key Features

- **High-Performance Thread Pool:** Creates a pool of worker threads to execute jobs, avoiding the high cost of creating and destroying threads for each task.
- **Asynchronous Task Submission:** The `submit` method is a fully generic template that can accept any function or lambda, along with its arguments.
- **Future-Based Results:** Submitting a task immediately returns a `std::future`, allowing the caller to retrieve the result later without blocking.
- **Task Dependency Management:** Easily create complex workflows where jobs only execute after their prerequisite tasks have been completed.
- **Header-Only Design:** For easy integration, simply add the `include` directory to your project's include path.
- **Modern & Tested:** Written in C++17 and thoroughly unit-tested using the GoogleTest framework.

## Real-World Use Cases

This library is ideal for any application that needs to perform multiple complex, time-consuming tasks without freezing or slowing down.

- **Game Development:** Parallelize physics calculations, AI decision-making, asset loading, and rendering logic.
- **Data & Scientific Computing:** Massively speed up image/video processing, financial modeling simulations, and large-scale data analysis by processing data chunks in parallel.
- **High-Performance Servers:** Handle thousands of concurrent client requests efficiently by offloading work to the thread pool, ensuring the main thread remains responsive.

## Basic Usage

Using the thread pool is simple. Create a pool, submit a job, and get the future back.

```cpp
#include <iostream>
#include "job_system/ThreadPool.h"

int main() {
    // Create a pool with 4 worker threads.
    ThreadPool pool(4);

    // Submit a simple lambda that returns a value.
    std::future<int> future_result = pool.submit([] {
        return 42;
    });

    // The .get() method waits for the task to finish and retrieves the result.
    int result = future_result.get();

    std::cout << "The result is: " << result << std::endl; // Prints "The result is: 42"

    return 0;
}
Advanced Usage (Task Dependencies)
You can easily create a chain of dependent tasks. The following example submits TaskB, which will only begin after TaskA has finished.

C++

#include <iostream>
#include <string>
#include <future>
#include "job_system/ThreadPool.h"

int main() {
    ThreadPool pool(4);
    std::cout << "Submitting tasks..." << std::endl;

    // Submit Task A
    auto future_A = pool.submit([] {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        return std::string("Hello");
    });

    // Submit Task B, which depends on Task A.
    // We move future_A into the new lambda's capture clause.
    auto future_B = pool.submit([future_A = std::move(future_A)]() mutable {
        // The first thing we do is wait for Task A's result.
        std::string result_from_A = future_A.get();
        return result_from_A + " from a Dependent Task!";
    });

    std::cout << "Main thread doing other work..." << std::endl;

    // Get the final result from the end of the dependency chain.
    std::string final_result = future_B.get();
    std::cout << "Final result: " << final_result << std::endl;

    return 0;
}
Building the Project
This project uses CMake for building the library, examples, and tests.

Bash

# 1. Clone the repository
git clone [https://github.com/Karthikgaur8/concurrent-job-system.git](https://github.com/Karthikgaur8/concurrent-job-system.git)
cd concurrent-job-system

# 2. Initialize submodules (for GoogleTest)
git submodule update --init --recursive

# 3. Create a build directory
mkdir build
cd build

# 4. Configure with CMake
# For Makefiles (Linux, macOS, Cygwin)
cmake .. -G "Unix Makefiles"
# For Visual Studio
# cmake ..

# 5. Build the project
make
Running Tests
The test suite is built automatically. To run the tests, navigate to the build directory and run ctest.

Bash

cd build
ctest --verbose
License
This project is licensed under the MIT License. See the LICENSE file for details.
