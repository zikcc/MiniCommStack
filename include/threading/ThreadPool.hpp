#pragma once
#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>

class ThreadPool {
public:
    // std::function<void()> 是一个模板类，表示一个可调用对象，可以接受0个或多个参数，并且返回void
    // 它继承自 std::function 类，可以像函数一样使用
    // using 是类型别名，可以给类型取一个别名   
    using Task = std::function<void()>;
    // explicit 是显式构造函数，禁止隐式转换
    // 它禁止了隐式转换，例如：ThreadPool pool = 4; 这样的语句会被禁止
    // 它要求必须显式地传入一个参数，例如：ThreadPool pool(4);
    // std::thread::hardware_concurrency() 是获取硬件支持的并发线程数
    // 例如：ThreadPool pool(std::thread::hardware_concurrency());
    // 这样就可以根据硬件支持的并发线程数来创建线程池   
    explicit ThreadPool(size_t threads = std::thread::hardware_concurrency());
    ~ThreadPool();

    // 向线程池提交任务
    // 它接受一个 Task 类型的参数，表示一个可调用对象，可以接受0个或多个参数，并且返回void
    // 例如：pool.enqueue([]{std::cout << "Hello, World!" << std::endl;});
    // 这样就可以向线程池提交一个任务，任务的内容是打印一句话   
    void enqueue(Task task);

private:
    std::vector<std::thread> workers;  // 工作线程
    std::queue<Task> tasks;    // 任务队列
    // std::mutex 是一个互斥锁，用于保护共享资源
    // 它是一个线程安全的锁，可以防止多个线程同时访问共享资源
    // 什么时候需要使用互斥锁？
    std::mutex mtx;
    // std::condition_variable 是一个条件变量，用于线程之间的同步
    // 它是一个条件变量，用于线程之间的同步
    std::condition_variable cv;
    // std::atomic 是一个原子变量，用于线程之间的同步
    // 它是一个原子变量，用于线程之间的同步
    std::atomic<bool> stop;
};
