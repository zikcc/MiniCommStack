#include "app/Server.hpp"

#include <arpa/inet.h>  // 包含IP地址转换函数
#include <fcntl.h>
#include <netinet/tcp.h>  // for TCP keepalive options
#include <string.h>
#include <sys/epoll.h>  // 包含epoll API
#include <unistd.h>

#include "net/Connection.hpp"
#include "utils/Logger.hpp"

Server::Server(const ServerConfig& config)
    : config(config),
      server_fd(-1),
      epoll_fd(-1),
      running(false),
      thread_pool(config.thread_pool_size) {
    utils::Logger::getInstance().setLogLevel(utils::LogLevel::INFO);
}

Server::~Server() { stop(); }

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
    // 后续所有错误路径需关闭 socket
    auto cleanup = [this]() {
        close(server_fd);
        server_fd = -1;
    };
    // 设置socket选项
    int opt = 1;
    // 设置socket选项 SO_REUSEADDR 允许在同一端口上多次绑定和启动服务器
    if (config.reuse_addr && setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
                                        &opt, sizeof(opt)) < 0) {
        LOG_ERROR("Failed to set SO_REUSEADDR: %s", strerror(errno));
        cleanup();  // 新增：关闭 socket
        return false;
    }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(config.port);

    // 绑定socket到本地地址和端口
    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        LOG_ERROR("Failed to bind: %s", strerror(errno));
        cleanup();  // 新增：关闭 socket
        return false;
    }

    // 监听连接请求
    if (listen(server_fd, config.backlog) < 0) {
        LOG_ERROR("Failed to listen: %s", strerror(errno));
        cleanup();  // 新增：关闭 socket
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
    // 设置事件类型为EPOLLIN，表示有数据可读(有新的连接到达)
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
    const int MAX_EVENTS = 1024;
    epoll_event events[MAX_EVENTS];

    LOG_INFO("Server main loop started");
    while (running) {
        /*
        主线程的事件循环（非阻塞）​
        */
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
        // 接收客户端连接
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        // 非阻塞模式​​：若没有新连接，accept4会立即返回-1并设置errno为EAGAIN/EWOULDBLOCK
        int client_fd = accept4(server_fd, (sockaddr*)&client_addr, &client_len,
                                SOCK_NONBLOCK);

        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;  // 没有更多连接
            LOG_ERROR("Failed to accept connection: %s", strerror(errno));
            continue;
        }

        // 设置客户端socket选项
        if (config.keep_alive) {
            int opt = 1;
            // 启用TCP保活机制
            setsockopt(client_fd, SOL_SOCKET, SO_KEEPALIVE, &opt, sizeof(opt));
            // 连接空闲多久后开始发送保活探测包（秒）
            setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPIDLE,
                       &config.keep_alive_time, sizeof(config.keep_alive_time));
            // 保活探测包发送间隔（秒）
            setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPINTVL,
                       &config.keep_alive_intvl,
                       sizeof(config.keep_alive_intvl));
            // 发送多少次未响应后断开连接
            setsockopt(client_fd, IPPROTO_TCP, TCP_KEEPCNT,
                       &config.keep_alive_probes,
                       sizeof(config.keep_alive_probes));
        }

        // 注册到epoll​
        epoll_event ev{};
        // 监听可读事件
        ev.events = EPOLLIN | EPOLLET;  // 边缘触发模式（只在状态变化时通知）
        /*
        边缘触发（ET）特性​​：
        必须在本次事件处理中​​完全读取所有数据​​
        适合高性能场景，但编程复杂度更高
        */
        ev.data.fd = client_fd;
        // epoll_ctl 管理epoll实例的核心系统调用
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) == -1) {
            LOG_ERROR("Failed to add client to epoll: %s", strerror(errno));
            close(client_fd);
            continue;
        }
        auto conn = std::make_shared<Connection>(client_fd, epoll_fd);
        conn_manager.addConnection(client_fd, conn);

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        LOG_INFO("New connection accepted: fd=%d, addr=%s:%d", client_fd, ip,
                 ntohs(client_addr.sin_port));
    }
}

void Server::handleClientEvent(int fd, uint32_t events) {
    auto conn = conn_manager.getConnection(fd);
    if (!conn) {
        cleanupConnection(fd);
        return;
    }
    if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
        cleanupConnection(fd);
        return;
    }
    // 传递 weak_ptr 到线程池
    std::weak_ptr<Connection> weak_conn = conn;
    // 使用线程池处理客户端事件
    thread_pool.enqueue([this, fd, events, weak_conn]() {
        // 在异步多线程模型中，主线程（处理事件循环）和工作线程（处理具体任务）存在竞态条件。可能在主线程获取连接后，任务进入线程池队列前，连接已被关闭。因此，在线程池任务内部需要再次检查连接状态。
        if (auto conn = weak_conn.lock()) {
            // EPOLLIN
            // 事件​​：表示套接字可读（有数据到达或连接关闭）。
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

        // 1. 关闭线程池（停止接受新任务）
        thread_pool.shutdown();

        // 2. 等待所有任务完成
        thread_pool.wait();

        // 3. 关闭所有连接和资源
        for (const auto& pair : conn_manager.getAllConnections()) {
            cleanupConnection(pair.first);
        }

        if (epoll_fd != -1) close(epoll_fd);
        if (server_fd != -1) close(server_fd);

        LOG_INFO("Server shutdown complete");
    }
}
