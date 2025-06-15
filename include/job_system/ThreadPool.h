#pragma once

#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

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

    template<typename F, typename... Args>
    auto submit(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
        // Calculate the function's return type
        using ReturnType = decltype(f(args...));

        // Create a packaged_task. A packaged_task needs a function that takes zero
        // arguments. So, we wrap our function and its arguments in a lambda.
        // This is a modern replacement for std::bind.
        auto task_ptr = std::make_shared<std::packaged_task<ReturnType()>>(
            [func = std::forward<F>(f), t_args = std::make_tuple(std::forward<Args>(args)...)]() mutable -> ReturnType {
                // std::apply invokes the captured function with arguments from the tuple
                return std::apply(func, std::move(t_args));
            }
        );

        std::future<ReturnType> result_future = task_ptr->get_future();
        {
            std::unique_lock<std::mutex> lock(queue_mutex_);

            if(stop_) {
                throw std::runtime_error("submit on stopped ThreadPool");
            }
            
            tasks_.emplace([task_ptr](){ (*task_ptr)(); });
        }

        condition_.notify_one();

        return result_future;
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