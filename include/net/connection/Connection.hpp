// Connection.hpp
#pragma once
#include <mutex>

#include "net/protocol/BaseProtocol.hpp"


class Connection {
   public:
    explicit Connection(int fd);
    Connection(int fd, int epoll_fd);
    ~Connection();

    // 禁止拷贝和移动
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
    Connection(Connection&&) = delete;
    Connection& operator=(Connection&&) = delete;

    int getFd() const { return fd_; }
    bool handleRead();   // 处理读事件（线程安全）
    bool handleWrite();  // 处理写事件（线程安全）

   private:
    const int fd_;              // 使用 const 防止意外修改
    int epoll_fd_;              // epoll 实例描述符
    BaseProtocol proto_;            // 协议处理器（内部管理发送/接收缓冲区）
    mutable std::mutex mutex_;  // 保护协议操作

    /** 根据 want_write 决定是否在 epoll 事件里加上 EPOLLOUT */
    void modifyEpollEvents(bool want_write);
};