// src/load_test/LoadTester.hpp
#pragma once

#include <string>
#include <vector>
#include <thread>
#include <atomic>

/// 并发压测工具
class LoadTester {
public:
    /// @param host               服务器 IP 或域名
    /// @param port               服务器端口
    /// @param num_threads        并发线程数
    /// @param messages_per_thread 每线程发送的消息数
    LoadTester(std::string host,
               uint16_t port,
               int num_threads,
               int messages_per_thread);

    /// 执行压测：阻塞直到全部线程结束
    void run();

private:
    /// 单个线程的工作函数
    void worker(int thread_index);

    // 参数
    std::string host_;
    uint16_t    port_;
    int         num_threads_;
    int         messages_per_thread_;

    // 统计
    std::atomic<int> success_count_{0};
    std::atomic<int> failure_count_{0};

    // 线程
    std::vector<std::thread> threads_;
};
