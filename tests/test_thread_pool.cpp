#include <gtest/gtest.h>
#include "job_system/ThreadPool.h"

// Test case to verify that the ThreadPool can be constructed and destructed.
TEST(ThreadPoolTest, ConstructionDestruction) {
    try {
        ThreadPool pool(4); // Create a pool with 4 threads
    } catch (const std::exception& e) {
        FAIL() << "ThreadPool construction or destruction threw an exception: " << e.what();
    }
    // The test passes if the ThreadPool object is created and destructed
    // without crashing or throwing an exception.
    SUCCEED();
}
// Test case to verify that a submitted task is executed.
TEST(ThreadPoolTest, SubmitsAndExecutesTask) {
    // We use std::atomic<bool> because this flag will be written to by one
    // thread and read by the main thread. std::atomic ensures this is safe.
    std::atomic<bool> task_executed = false;

    {
        ThreadPool pool(2); // Create a pool with 2 threads.

        // Submit a simple lambda function as a task.
        // This lambda will set our flag to true.
        pool.submit([&task_executed] {
            task_executed.store(true);
        });

        // The pool goes out of scope here. The destructor will be called,
        // which waits for all tasks to complete before shutting down.
    }

    // By the time the pool is destroyed, the task must have been executed.
    EXPECT_TRUE(task_executed.load());
}