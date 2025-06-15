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

// Test case to verify that a task can return a value.
TEST(ThreadPoolTest, SubmitsTaskAndRetrievesResult) {
    ThreadPool pool(2);

    // Submit a lambda that returns an integer.
    auto future_result = pool.submit([] {
        // Simulate some work
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        return 42;
    });

    // The .get() method will wait until the task is complete and then
    // return the value. It's how you "redeem the ticket".
    int result = future_result.get();

    // Check if we received the correct value.
    EXPECT_EQ(result, 42);
}

// A simple standalone function to test argument passing
int add(int a, int b) {
    return a + b;
}

// Test case to verify that tasks with arguments can be submitted.
TEST(ThreadPoolTest, SubmitsTaskWithArguments) {
    ThreadPool pool(2);

    // Submit the 'add' function with arguments 10 and 20.
    auto future_result = pool.submit(add, 10, 20);

    int result = future_result.get();

    EXPECT_EQ(result, 30);
}

// Test case to verify that task dependencies are handled correctly.
TEST(ThreadPoolTest, HandlesTaskDependencies) {
    ThreadPool pool(4);
    std::vector<int> execution_log;
    std::mutex log_mutex; // Mutex to protect the execution log

    // Task A: Returns an int.
    auto future_A = pool.submit([] {
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
        return 1;
    });

    // Task B: Depends on Task A. It takes Task A's future as an argument.
    // We move the future into the lambda to transfer ownership.
    auto future_B = pool.submit([&log_mutex, &execution_log, future_A = std::move(future_A)]() mutable {
    // Wait for future from Task A to be ready, then get its value.
    int result_A = future_A.get();
    
    // Log our execution order
    std::lock_guard<std::mutex> lock(log_mutex);
    execution_log.push_back(result_A);
    execution_log.push_back(2);
});
    // The main thread will wait here until Task B is complete.
    future_B.get();

    // Verify that the tasks executed in the correct order.
    ASSERT_EQ(execution_log.size(), 2);
    EXPECT_EQ(execution_log[0], 1); // Result from Task A should be first
    EXPECT_EQ(execution_log[1], 2); // Result from Task B should be second
}