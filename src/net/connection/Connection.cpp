#include "net/connection/Connection.hpp"

#include <sys/epoll.h>  // 包含epoll API

#include <chrono>
#include <cstring>

#include "utils/Logger.hpp"
#include "utils/Metrics.hpp"

Connection::Connection(int fd, int epfd)
    : fd_(fd), epoll_fd_(epfd), proto_(fd) {
    Metrics::getInstance().incrementConnections();
    LOG_INFO("New connection created: fd=%d", fd);
}

Connection::~Connection() {
    if (fd_ >= 0) {
        Metrics::getInstance().decrementConnections();
        close(fd_);
        LOG_INFO("Connection closed: fd=%d", fd_);
    }
}

void Connection::modifyEpollEvents(bool want_write) {
    epoll_event ev{};
    ev.data.fd = fd_;
    ev.events = EPOLLIN | EPOLLET;
    if (want_write) {
        ev.events |= EPOLLOUT;
    }
    if (epoll_ctl(epoll_fd_, EPOLL_CTL_MOD, fd_, &ev) < 0) {
        // 如果 fd 已经被关闭或不在 epoll 中，就不处理
        if (errno == EBADF || errno == ENOENT) {
            return;
        }
        LOG_ERROR("modifyEpollEvents failed fd=%d: %s", fd_, strerror(errno));
    }
}

bool Connection::handleRead() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto start = std::chrono::high_resolution_clock::now();

    try {
        bool keep_reading = true;
        while (keep_reading) {
            Packet request;
            auto status = proto_.tryReceivePacket(request);
            if (status == Protocol::ReadStatus::OK) {
                // 处理数据...
                Metrics::getInstance().incrementBytesReceived(request.length +
                                                              8);
                Metrics::getInstance().incrementRequests();

                // 生成响应（示例：简单回显）
                Packet response;
                response.header = 0xABCD;
                response.payload = "Server received: " + request.payload;
                response.length = response.payload.length();
                response.checksum = calculate_checksum(std::vector<uint8_t>(
                    response.payload.begin(), response.payload.end()));

                // 将响应加入协议层发送队列（非立即发送）
                proto_.enqueuePacket(response);
                if (proto_.hasPendingSendData()) {
                    modifyEpollEvents(true);
                }
                // 记录处理延迟
                auto end = std::chrono::high_resolution_clock::now();
                auto duration =
                    std::chrono::duration_cast<std::chrono::microseconds>(
                        end - start);
                Metrics::getInstance().recordLatency(duration.count());
            } else if (status == Protocol::ReadStatus::NeedRetry) {
                break;  // 数据未就绪
            } else {
                return false;  // 错误或连接关闭
            }
        }
        return true;
    } catch (const std::exception &e) {
        LOG_ERROR("Failed to handle read on fd=%d: %s", fd_, e.what());
        Metrics::getInstance().incrementErrors();
        return false;
    }
}

bool Connection::handleWrite() {
    std::lock_guard<std::mutex> lock(mutex_);
    auto start = std::chrono::high_resolution_clock::now();

    try {
        int saved_errno = 0;
        bool success = proto_.flushSendBuffer(
            saved_errno);  // 修改 Protocol 方法以返回错误码
        // 尝试刷新发送缓冲区
        if (!success) {
            // 非阻塞模式下 EAGAIN 是正常情况，无需记录错误
            if (saved_errno != EAGAIN && saved_errno != EWOULDBLOCK) {
                Metrics::getInstance().incrementErrors();
                LOG_ERROR("Failed to send data on fd=%d", fd_);
            }
            return false;
        }
        // 只有当 send_buffer_ 真正清空后，才去掉 EPOLLOUT
        if (!proto_.hasPendingSendData()) {
            modifyEpollEvents(false);  // 只剩 EPOLLIN | EPOLLET
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto duration =
            std::chrono::duration_cast<std::chrono::microseconds>(end - start);
        Metrics::getInstance().recordLatency(duration.count());

        return true;
    } catch (const std::exception &e) {
        LOG_ERROR("Failed to handle write on fd=%d: %s", fd_, e.what());
        Metrics::getInstance().incrementErrors();
        return false;
    }
}
