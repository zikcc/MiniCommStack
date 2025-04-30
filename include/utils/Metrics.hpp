#pragma once
#include <atomic>
#include <chrono>
#include <string>
#include <unordered_map>
#include <mutex>

class Metrics {
public:
    static Metrics& getInstance() {
        static Metrics instance;
        return instance;
    }

    // 连接统计
    void incrementConnections() { ++total_connections_; }
    void decrementConnections() { --current_connections_; }
    uint64_t getTotalConnections() const { return total_connections_; }
    uint64_t getCurrentConnections() const { return current_connections_; }

    // 请求统计
    void incrementRequests() { ++total_requests_; }
    void incrementBytesReceived(uint64_t bytes) { bytes_received_ += bytes; }
    void incrementBytesSent(uint64_t bytes) { bytes_sent_ += bytes; }
    uint64_t getTotalRequests() const { return total_requests_; }
    uint64_t getBytesReceived() const { return bytes_received_; }
    uint64_t getBytesSent() const { return bytes_sent_; }

    // 错误统计
    void incrementErrors() { ++total_errors_; }
    uint64_t getTotalErrors() const { return total_errors_; }

    // 延迟统计
    void recordLatency(uint64_t microseconds) {
        std::lock_guard<std::mutex> lock(latency_mutex_);
        total_latency_ += microseconds;
        ++latency_samples_;
    }
    double getAverageLatency() const {
        std::lock_guard<std::mutex> lock(latency_mutex_);
        return latency_samples_ > 0 ? static_cast<double>(total_latency_) / latency_samples_ : 0.0;
    }

    // 重置统计
    void reset() {
        total_connections_ = 0;
        current_connections_ = 0;
        total_requests_ = 0;
        bytes_received_ = 0;
        bytes_sent_ = 0;
        total_errors_ = 0;
        total_latency_ = 0;
        latency_samples_ = 0;
    }

private:
    Metrics() = default;
    ~Metrics() = default;
    Metrics(const Metrics&) = delete;
    Metrics& operator=(const Metrics&) = delete;

    std::atomic<uint64_t> total_connections_{0};
    std::atomic<uint64_t> current_connections_{0};
    std::atomic<uint64_t> total_requests_{0};
    std::atomic<uint64_t> bytes_received_{0};
    std::atomic<uint64_t> bytes_sent_{0};
    std::atomic<uint64_t> total_errors_{0};
    
    mutable std::mutex latency_mutex_;
    uint64_t total_latency_{0};
    uint64_t latency_samples_{0};
}; 