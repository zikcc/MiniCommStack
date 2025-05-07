#include "threading/ThreadPool.hpp"

#include <mutex>

ThreadPool::ThreadPool(size_t threads) : stop(false) {
    // 创建线程池中的线程
    for (size_t i = 0; i < threads; ++i) {
        workers.emplace_back([this] {
            while (true) {
                Task task;
                {
                    // 加锁
                    std::unique_lock<std::mutex> lock(mtx);
                    cv.wait(lock, [this] { return stop || !tasks.empty(); });
                    // 如果线程池停止并且任务队列为空，则退出循环
                    if (stop && tasks.empty()) return;
                    // 从任务队列中获取任务
                    task = std::move(tasks.front());
                    // 从任务队列中移除任务
                    tasks.pop();
                }
                task();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    shutdown();
    wait();
}

void ThreadPool::shutdown() {
    {
        std::unique_lock<std::mutex> lock(mtx);
        stop = true;
    }
    cv.notify_all();
}

void ThreadPool::wait() {
    for (auto &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::enqueue(Task task) {
    {
        std::lock_guard<std::mutex> lock(mtx);
        if (stop) {
            throw std::runtime_error("Enqueue on stopped ThreadPool");
        }
        // 将任务添加到任务队列中
        tasks.push(std::move(task));
    }
    // notify_one 函数会唤醒一个等待的线程
    cv.notify_one();
}