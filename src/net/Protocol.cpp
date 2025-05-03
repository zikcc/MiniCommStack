#include "net/Protocol.hpp"
#include <unistd.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

// 构造函数：初始化 socket 描述符和缓冲区
Protocol::Protocol(int socket_fd) 
    : sockfd_(socket_fd)          // 初始化列表优于函数体内赋值
    , send_buffer_()              // 显式初始化（可选，但提高可读性）
    , recv_buffer_() {
    if (socket_fd < 0) {
        throw std::invalid_argument("Invalid socket descriptor"); // 参数合法性检查
    }
}

// 析构函数：无需关闭 socket（由 Connection 类管理）
Protocol::~Protocol() {
    // 清空缓冲区（实际可省略，vector 析构时会自动释放）
    send_buffer_.clear();
    recv_buffer_.clear();
}

// 将接收到的数据包放入发送缓存区
void Protocol::enqueuePacket(const Packet& pkt) {
    auto data = pkt.serialize();
    send_buffer_.insert(send_buffer_.end(), data.begin(), data.end());
}

bool Protocol::flushSendBuffer() {
    size_t total_sent = 0;
    while (total_sent < send_buffer_.size()) {
        ssize_t sent = write(sockfd_, send_buffer_.data() + total_sent, 
                            send_buffer_.size() - total_sent);
        if (sent == -1) {
            if (errno == EAGAIN) break;  // 非阻塞模式正常退出
            return false;  // 真实错误
        }
        total_sent += sent;
    }
    send_buffer_.erase(send_buffer_.begin(), send_buffer_.begin() + total_sent);
    return true;
}

bool Protocol::tryReceivePacket(Packet& pkt) {
    // 尝试从接收缓冲区解析包
    if (parseFromBuffer(recv_buffer_, pkt)) {
        return true;
    }

    // 从Socket读取新数据到缓冲区
    uint8_t buf[4096];
    ssize_t received = read(sockfd_, buf, sizeof(buf));
    
    if (received > 0) {
        recv_buffer_.insert(recv_buffer_.end(), buf, buf + received);
        // 再次尝试解析（可能有新数据到达）
        return parseFromBuffer(recv_buffer_, pkt);
    } else if (received == 0) {
        return false; // 连接关闭
    } else {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return false; // 非阻塞模式下数据未就绪
        }
        return false; // 真实错误
    }
}

bool Protocol::parseFromBuffer(std::vector<uint8_t>& buffer, Packet& pkt) {
    // 包头长度不足，无法解析
    if (buffer.size() < 6) return false;

    // 解析 payload 长度
    uint32_t network_length;
    memcpy(&network_length, buffer.data() + 2, 4); // 从第3字节开始取4字节
    uint32_t payload_length = ntohl(network_length);
    size_t total_needed = 6 + payload_length + 2; // 包头+payload+校验和

    // 检查缓冲区是否足够
    if (buffer.size() < total_needed) return false;

    // 提取完整包数据
    std::vector<uint8_t> packet_data(buffer.begin(), buffer.begin() + total_needed);
    pkt = Packet::deserialize(packet_data);

    // 从缓冲区删除已处理数据
    buffer.erase(buffer.begin(), buffer.begin() + total_needed);
    return true;
}

