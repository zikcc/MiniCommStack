// src/load_test/LoadTester.cpp
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <ostream>

#include "load_test/LoadTester.hpp"
#include "net/Packet.hpp"
#include "net/Protocol.hpp"

LoadTester::LoadTester(std::string host, uint16_t port, int num_threads,
                       int messages_per_thread)
    : host_(std::move(host)),
      port_(port),
      num_threads_(num_threads),
      messages_per_thread_(messages_per_thread) {}

void LoadTester::run() {
    threads_.reserve(num_threads_);
    auto t0 = std::chrono::steady_clock::now();

    // 启动线程
    for (int i = 0; i < num_threads_; ++i) {
        threads_.emplace_back(&LoadTester::worker, this, i);
    }
    // 等待完成
    for (auto& t : threads_) t.join();

    auto t1 = std::chrono::steady_clock::now();
    double secs = std::chrono::duration<double>(t1 - t0).count();
    int total = num_threads_ * messages_per_thread_;

    // 打印结果
    std::cout << "Total messages: " << total << std::endl;
    std::cout << "Success: " << success_count_.load() 
              << ", Failure: " << failure_count_.load() << std::endl;
    std::cout << "Elapsed: " << secs << ", Throughput: " << total / secs
              << " msg/s" << std::endl;
}

void LoadTester::worker(int thread_index) {
    // 1) 建立连接
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ++failure_count_;
        return;
    }
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port_);
    if (::inet_pton(AF_INET, host_.c_str(), &servaddr.sin_addr) != 1 ||
        ::connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        ::close(sockfd);
        ++failure_count_;
        return;
    }

    Protocol proto(sockfd);
    for (int i = 0; i < messages_per_thread_; ++i) {
        // 构造并发送 Packet
        std::string payload = "msg from thread " +
                              std::to_string(thread_index) + "#" +
                              std::to_string(i);
        Packet pkt;
        pkt.header = 0xABCD;
        pkt.payload = payload;
        pkt.length = static_cast<uint32_t>(payload.size());
        pkt.checksum = calculate_checksum(
            std::vector<uint8_t>(payload.begin(), payload.end()));
        proto.enqueuePacket(pkt);

        // flush 发送缓冲区
        int err = 0;
        while (proto.hasPendingSendData()) {
            if (!proto.flushSendBuffer(err) && err != EAGAIN &&
                err != EWOULDBLOCK) {
                ++failure_count_;
                break;
            }
        }

        // 接收回显
        Packet resp;
        while (true) {
            auto status = proto.tryReceivePacket(resp);
            if (status == Protocol::ReadStatus::OK) {
                // std::cout << "Echo> " << resp.payload << "\n";
                ++success_count_;
                break;
            }
            if (status == Protocol::ReadStatus::Error) {
                ++failure_count_;
                break;
            }
            // NeedRetry 继续循环 recv
        }
    }

    ::close(sockfd);
}
