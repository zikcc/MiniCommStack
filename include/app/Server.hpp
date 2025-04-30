// Server.h
#ifndef SERVER_H
#define SERVER_H

#include "net/ConnectionManager.hpp"
#include "threading/ThreadPool.hpp"
#include "app/ServerConfig.hpp"
#include <atomic>

class Server {
public:
    // explicit 禁止隐式转换
    explicit Server(const ServerConfig& config = ServerConfig());
    ~Server();
    
    bool setup();         // 初始化监听 socket 和 epoll
    void run();           // 主循环
    void stop();          // 优雅关闭

private:
    ServerConfig config;
    // 监听 socket 和 epoll 
    int server_fd;
    int epoll_fd;
    // 运行状态
    // atomic 原子变量 用于多线程共享变量 
    // 什么是原子变量？
    // 原子变量是一种特殊的变量，它可以在多线程环境下安全地进行读写操作，而不需要加锁。
    // 原子变量的操作是原子性的，即要么全部执行，要么全部不执行，不会被其他线程中断。 
    std::atomic<bool> running;
    // 连接管理
    ConnectionManager conn_manager;
    // 线程池
    ThreadPool thread_pool;
    
    // 设置 socket 和 epoll
    bool setupSocket();
    bool setupEpoll();
    // 处理新连接
    void handleNewConnection();
    // 处理客户端事件
    void handleClientEvent(int fd, uint32_t events);
    // 清理连接
    void cleanupConnection(int fd);
};

#endif // SERVER_H
