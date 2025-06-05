// UdpProtocol.cpp
#include "net/UdpProtocol.hpp"
#include <unistd.h>
#include <stdexcept>
#include <sys/socket.h>

UdpProtocol::UdpProtocol(int socket_fd, sockaddr_in peer_addr)
    : sockfd_(socket_fd), peer_(peer_addr) {
        if (socket_fd < 0) {
            throw std::invalid_argument(
                "Invalid socket descriptor");  // 参数合法性检查
        }
    }

UdpProtocol::~UdpProtocol() {
    if (sockfd_ >= 0) close(sockfd_);
}

BaseProtocol::ReadStatus UdpProtocol::tryReceivePacket(Packet& pkt) {
    uint8_t buffer[4096];
    socklen_t len = sizeof(peer_);
    ssize_t received = recvfrom(sockfd_, buffer, sizeof(buffer), 0,
                                (sockaddr*)&peer_, &len);
    if (received > 0) {
        std::vector<uint8_t> data(buffer, buffer + received);
        pkt = Packet::deserialize(data);
        return ReadStatus::OK;
    } else if (received == 0 || (received < 0 && errno != EAGAIN && errno != EWOULDBLOCK)) {
        return ReadStatus::Error;
    } else {
        return ReadStatus::NeedRetry;
    }
}

void UdpProtocol::enqueuePacket(const Packet& pkt) {
    send_buffer_ = pkt.serialize(); // UDP 每次发送一个完整包
}

bool UdpProtocol::flushSendBuffer(int& saved_errno) {
    if (send_buffer_.empty()) return true;
    ssize_t sent = sendto(sockfd_, send_buffer_.data(), send_buffer_.size(), 0,
                          (sockaddr*)&peer_, sizeof(peer_));
    if (sent < 0) {
        saved_errno = errno;
        return false;
    }
    send_buffer_.clear();
    return true;
}

bool UdpProtocol::hasPendingSendData() const {
    return !send_buffer_.empty();
}
