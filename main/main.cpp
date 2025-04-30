// main.cpp
#include "app/Server.hpp"
#include "app/ServerConfig.hpp"

int main() {
    ServerConfig config;
    config.port = 8888;  // 设置端口
    Server server(config);
    server.run();         // 启动主循环
    return 0;
}
