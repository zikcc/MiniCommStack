#include "app/Server.hpp"
#include "net/Connection.hpp"
#include "utils/Logger.hpp"
#include <arpa/inet.h>   // 包含IP地址转换函数
#include <sys/epoll.h>   // 包含epoll API
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <netinet/tcp.h>  // for TCP keepalive options

Server::Server(const ServerConfig& config) 
    : config(config), 
      server_fd(-1), 
      epoll_fd(-1), 
      running(false),
      thread_pool(config.thread_pool_size) {
    utils::Logger::getInstance().setLogLevel(utils::LogLevel::INFO);
}

Server::~Server() {
    stop();
}

bool Server::setup() {
    if (!setupSocket()) {
        return false;
    }
    if (!setupEpoll()) {
        close(server_fd);
        return false;
    }
    running = true;
    LOG_INFO("Server started on port %d", config.port);
    return true;
}

// 启动服务器
bool Server::setupSocket() {
    // 创建socket
    // AF_INET: IPv4地址族
    // SOCK_STREAM: 流式套接字，提供可靠的、面向连接的数据传输服务
    // SOCK_NONBLOCK: 非阻塞模式，套接字操作不会阻塞，而是立即返回
    // 0: 默认协议  
    server_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    if (server_fd == -1) {
        // errno 是系统错误码   
        LOG_ERROR("Failed to create socket: %s", strerror(errno));
        return false;
    }

    // 设置socket选项
    int opt = 1;
    // 设置socket选项 SO_REUSEADDR 允许在同一端口上多次绑定和启动服务器
    if (config.reuse_addr && setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        LOG_ERROR("Failed to set SO_REUSEADDR: %s", strerror(errno));
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(config.port);

    // 绑定socket到本地地址和端口
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to bind: %s", strerror(errno));
        return false;
    }

    // 监听连接请求
    if (listen(server_fd, config.backlog) < 0) {
        LOG_ERROR("Failed to listen: %s", strerror(errno));
        return false;
    }

    LOG_INFO("Server listening on port %d", config.port);
    return true;
}

bool Server::setupEpoll() {
    // 创建epoll实例    
    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        LOG_ERROR("Failed to create epoll: %s", strerror(errno));
        return false;
    }

    // 添加服务器socket到epoll
    epoll_event ev{};
    // 设置事件类型为EPOLLIN，表示有数据可读
    ev.events = EPOLLIN;
    // 设置事件关联的文件描述符为服务器socket   
    ev.data.fd = server_fd;
    // 添加服务器socket到epoll
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev) == -1) {
        LOG_ERROR("Failed to add server socket to epoll: %s", strerror(errno));
        return false;
    }

    return true;
}

void Server::run() {
    // 创建epoll事件数组
    // epoll_event 是epoll的结构体，用于存储epoll事件
    // epoll_event 结构体定义如下：
    // struct epoll_event {
    //     uint32_t events; // 事件类型
    //     epoll_data_t data; // 事件关联的数据
    // };   
    // epoll_event 结构体中的 events 字段表示事件类型，data 字段表示事件关联的数据
    // epoll_event 结构体中的 events 字段可以表示以下事件类型：
    // EPOLLIN: 表示有数据可读
    // EPOLLOUT: 表示有数据可写
    // EPOLLERR: 表示有错误发生
    // EPOLLHUP: 表示有挂起事件
    // EPOLLRDHUP: 表示有读半关闭事件   
    const int MAX_EVENTS = 1024;
    epoll_event events[MAX_EVENTS];

    LOG_INFO("Server main loop started");
    while (running) {
        // 等待事件发生
        // 返回值：
        // 成功：返回事件发生的文件描述符个数
        // 失败：返回-1，并设置errno    
        int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (n == -1) {
            if (errno == EINTR) continue;  // 被信号中断，继续等待
            LOG_ERROR("epoll_wait error: %s", strerror(errno));
            break;
        }

        for (int i = 0; i < n; ++i) {
            // 处理服务器socket事件 
            if (events[i].data.fd == server_fd) {
                handleNewConnection();
            } else {
                // 处理客户端socket事件
                handleClientEvent(events[i].data.fd, events[i].events);
            }
        }
    }
    LOG_INFO("Server main loop stopped");
}

void Server::handleNewConnection() {
    while (true) {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept4(server_fd, (sockaddr*)&client_addr, &client_len, SOCK_NONBLOCK);
        
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;  // 没有更多连接
            LOG_ERROR("Failed to accept connection: %s", strerror(errno));
            continue;
        }

        // 设置客户端socket选项
        if (config.keep_alive) {
            int opt = 1;
            setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
            setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPIDLE, &config.keep_alive_time, sizeof(config.keep_alive_time));
            setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPINTVL, &config.keep_alive_intvl, sizeof(config.keep_alive_intvl));
            setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPCNT, &config.keep_alive_probes, sizeof(config.keep_alive_probes));
        }

        // 添加到epoll
        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET;  // 边缘触发
        ev.data.fd = client_fd;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
            LOG_ERROR("Failed to add client to epoll: %s", strerror(errno));
            close(client_fd);
            continue;
        }

        auto conn = std::make_shared<Connection>(client_fd);
        conn_manager.addConnection(client_fd, conn);
        LOG_INFO("New connection accepted: fd=%d, addr=%s:%d", 
                 client_fd, 
                 inet_ntoa(client_addr.sin_addr), 
                 ntohs(client_addr.sin_port));
    }
}

void Server::handleClientEvent(int fd, uint32_t events) {
    auto conn = conn_manager.getConnection(fd);
    if (!conn) {
        cleanupConnection(fd);
        return;
    }

    // 使用线程池处理客户端事件
    thread_pool.enqueue([this, fd, events]() {
        auto conn = conn_manager.getConnection(fd);
        if (!conn) return;

        if (events & EPOLLIN) {
            if (!conn->handleRead()) {
                LOG_INFO("Client disconnected: fd=%d", fd);
                cleanupConnection(fd);
            }
        }
        if (events & EPOLLOUT) {
            if (!conn->handleWrite()) {
                LOG_INFO("Client write error: fd=%d", fd);
                cleanupConnection(fd);
            }
        }
    });
}

void Server::cleanupConnection(int fd) {
    conn_manager.removeConnection(fd);
    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, nullptr);
    close(fd);
}

void Server::stop() {
    if (running) {
        LOG_INFO("Server shutting down...");
        running = false;
        
        // 等待所有连接处理完成
        thread_pool.~ThreadPool();
        
        // 关闭所有连接
        for (const auto& pair : conn_manager.getAllConnections()) {
            cleanupConnection(pair.first);
        }
        
        if (epoll_fd != -1) close(epoll_fd);
        if (server_fd != -1) close(server_fd);
        
        LOG_INFO("Server shutdown complete");
    }
}

