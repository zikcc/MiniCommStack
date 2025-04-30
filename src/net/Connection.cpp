#include "net/Connection.hpp"
#include "utils/Metrics.hpp"
#include "utils/Logger.hpp"
#include <iostream>
#include <cstring>
#include <chrono>

Connection::Connection(int fd_) : fd(fd_), proto(fd_) {
    Metrics::getInstance().incrementConnections();
    LOG_INFO("New connection created: fd=%d", fd);
}

Connection::~Connection() {
    if (fd >= 0) {
        Metrics::getInstance().decrementConnections();
        close(fd);
        LOG_INFO("Connection closed: fd=%d", fd);
    }
}

int Connection::getFd() const {
    return fd;
}

bool Connection::handleRead() {
    auto start = std::chrono::high_resolution_clock::now();
    
    Packet request;
    if (!proto.receive_packet(request)) {
        Metrics::getInstance().incrementErrors();
        LOG_ERROR("Failed to receive packet from fd=%d", fd);
        return false;
    }

    // 记录接收的字节数
    Metrics::getInstance().incrementBytesReceived(request.length + 8);  // 8 = header(2) + length(4) + checksum(2)
    Metrics::getInstance().incrementRequests();

    // 处理收到的数据包（这里简单回显）
    Packet response;
    response.header = 0xABCD;
    response.payload = "Server received: " + request.payload;
    response.length = response.payload.length();
    response.checksum = calculate_checksum(
        std::vector<uint8_t>(response.payload.begin(), response.payload.end())
    );

    // 将响应加入待发送队列
    pending_packets.push_back(response);
    
    // 记录处理延迟
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Metrics::getInstance().recordLatency(duration.count());
    
    // 尝试立即发送
    return handleWrite();
}

bool Connection::handleWrite() {
    auto start = std::chrono::high_resolution_clock::now();
    
    while (!pending_packets.empty()) {
        if (!proto.send_packet(pending_packets.front())) {
            Metrics::getInstance().incrementErrors();
            LOG_ERROR("Failed to send packet to fd=%d", fd);
            return false;
        }
        
        // 记录发送的字节数
        Metrics::getInstance().incrementBytesSent(pending_packets.front().length + 8);
        pending_packets.erase(pending_packets.begin());
    }
    
    // 记录发送延迟
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    Metrics::getInstance().recordLatency(duration.count());
    
    return true;
}
