#include "threading/ThreadPool.hpp"
#include <mutex>
// 线程池的实现
// ThreadPool::ThreadPool(size_t threads)是什么意思
// ThreadPool::ThreadPool(size_t threads)是ThreadPool类的构造函数
// size_t threads是构造函数的参数
// threads是线程池中的线程数
// : stop(false) 是初始化列表，用于初始化成员变量   
// stop是成员变量，用于表示线程池是否停止
// false是初始值，表示线程池未停止
ThreadPool::ThreadPool(size_t threads) : stop(false) {
    // 创建线程池中的线程
    for (size_t i = 0; i < threads; i++) {
        // emplace_back 是 C++ 标准库中的一个函数，用于在容器的末尾添加一个元素
        // 这里是在 workers 容器的末尾添加一个 lambda 表达式
        // 这个 lambda 表达式会不断地从任务队列中获取任务并执行 
        // 为什么要捕获 this？
        // 因为 lambda 表达式需要访问 ThreadPool 类的成员变量和成员函数
        // 捕获 this 可以使得 lambda 表达式可以访问 ThreadPool 类的成员变量和成员函数   
        workers.emplace_back([this] {
                while (true) {
                    // 为什么要实现task？
                    // task是任务，由 ThreadPool 类管理
                    Task task;
                    {
                        // 加锁 
                        std::unique_lock<std::mutex> lock(mtx);
                        // 等待条件变量
                        // wait 函数会阻塞当前线程，直到条件变量被唤醒 
                        // wait() 函数会释放锁，并阻塞当前线程，直到条件变量被唤醒
                        // 当条件变量被唤醒时，wait() 函数会重新加锁，并返回    
                        // 当工作线程调用 cv.wait 时，它会先检查传入的 lambda 表达式的返回值。
                        // 如果返回值为 true，说明条件已经满足，线程将继续执行后面的代码。如果返回值为 false，
                        // 线程将释放互斥锁 lock（避免阻塞其他线程对共享资源的访问），然后进入等待状态。
                        cv.wait(lock, [this] {
                            return stop || !tasks.empty();
                        }); 
                        // 如果线程池停止并且任务队列为空，则退出循环
                        if (stop && tasks.empty()) return;
                        // 从任务队列中获取任务
                        task = std::move(tasks.front());
                        // 从任务队列中移除任务
                        tasks.pop();
                    }
                    task();
                }
            }
        );
    }
}

ThreadPool::~ThreadPool() {
    stop = true;
    // notify_all 函数会唤醒所有等待的线程
    cv.notify_all();
    // 等待所有工作线程结束 
    for (auto &worker : workers) {
        // joinable 函数会检查线程是否可以被 join
        // join 函数会阻塞当前线程，直到线程结束    
        if (worker.joinable()) worker.join();   
    }
}

void ThreadPool::enqueue(Task task) {
    {
        // 加锁
        // 为什么加锁？
        // 因为任务队列是共享资源，需要加锁保护 
        // 这个代码流程是啥？
        // 1. 加锁
        // 2. 将任务添加到任务队列中
        // 3. 解锁
        // 4. 唤醒一个等待的线程    
        std::lock_guard<std::mutex> lock(mtx);
        // 将任务添加到任务队列中
        tasks.push(std::move(task));
    }
    // notify_one 函数会唤醒一个等待的线程
    cv.notify_one();
}