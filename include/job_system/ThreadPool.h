#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {
public:
    ThreadPool(size_t num_threads) {
        // Start the worker threads
        for(size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                // This is the main loop for each worker thread.
                while(true) {
                    std::function<void()> task;

                    // Use a block to scope the lock
                    {
                        // Acquire the lock on the queue
                        std::unique_lock<std::mutex> lock(this->queue_mutex_);

                        // Wait until the condition is met:
                        // 1. The pool is stopped.
                        // 2. There is a task in the queue.
                        this->condition_.wait(lock, [this] {
                            return this->stop_ || !this->tasks_.empty();
                        });

                        // If the pool is stopped and the queue is empty, exit the loop.
                        if(this->stop_ && this->tasks_.empty()) {
                            return;
                        }

                        // Pop a task from the queue
                        task = std::move(this->tasks_.front());
                        this->tasks_.pop();
                    } // The lock is released here

                    // Execute the task
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        // Use a block to scope the lock
        {
            // Acquire the lock
            std::unique_lock<std::mutex> lock(queue_mutex_);
            // Set the stop flag to true
            stop_ = true;
        } // The lock is released here

        // Notify all waiting threads
        condition_.notify_all();

        // Wait for all threads to complete their execution
        for(std::thread &worker : workers_) {
            worker.join();
        }
    }
    void submit(std::function<void()> task) {
        // Use a block to scope the lock
        {
            // Acquire the lock on the queue
            std::unique_lock<std::mutex> lock(queue_mutex_);

            // Add the task to the queue
            tasks_.push(std::move(task));
        } // The lock is released here

        // Notify one waiting thread that a task is available.
        // We use notify_one() because there's no need to wake up all
        // threads when only one can take the task.
        condition_.notify_one();
    }
private:
    // A vector to hold the worker threads.
    std::vector<std::thread> workers_;

    // The queue of tasks (jobs) to be executed.
    // std::function allows us to store any callable object (like lambdas).
    std::queue<std::function<void()>> tasks_;

    // Mutex for synchronizing access to the tasks queue.
    std::mutex queue_mutex_;

    // Condition variable for signaling between threads.
    std::condition_variable condition_;

    // A flag to signal the threads to stop.
    bool stop_ = false;
};