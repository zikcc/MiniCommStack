// main.cpp
#include <csignal>
#include <iostream>

#include "app/Server.hpp"
#include "app/ServerConfig.hpp"

static Server* g_server = nullptr;

// SIGINT/SIGTERM 信号处理：让 run() 退出
void handleSignal(int) {
    if (g_server) {
        g_server->stop();
    }
}

int main() {
    // 1) 配置
    ServerConfig config;
    config.port = 8888;
    config.backlog = 128;
    config.thread_pool_size = 4;
    config.reuse_addr = true;
    config.keep_alive = true;
    config.keep_alive_time = 60;
    config.keep_alive_intvl = 10;
    config.keep_alive_probes = 5;

    // 2) 创建 Server
    Server server(config);
    g_server = &server;

    // 3) 注册信号，优雅退出
    std::signal(SIGINT, handleSignal);
    std::signal(SIGTERM, handleSignal);

    // 4) 初始化资源
    if (!server.setup()) {
        std::cerr << "Server setup failed\n";
        return 1;
    }

    // 5) 进入主循环（阻塞）
    server.run();

    // 6) run() 返回后（如收到 stop()），析构过程中自动调用 stop()
    std::cout << "Server exiting\n";
    return 0;
}
