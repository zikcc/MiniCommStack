#include "net/Packet.hpp"
#include "net/Protocol.hpp"
#include "crawler/Crawler.hpp"
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string>
#include <cstring>
#include <sstream>

int main() {
    // 创建socket
    // socket 函数是 POSIX 标准库中的一个函数，用于创建一个 socket
    // 它的原型是：int socket(int domain, int type, int protocol);
    // domain 是协议族，AF_INET 表示 IPv4 协议族
    // type 是套接字类型，SOCK_STREAM 表示流式套接字
    // 还有什么类型？
    // 还有数据报套接字，SOCK_DGRAM 表示数据报套接字
    // 为什么用流式套接字？ 
    // 因为流式套接字是面向连接的，数据报套接字是面向无连接的
    // 什么是面向连接的？
    // 面向连接的通信方式，需要先建立连接，然后才能进行通信
    // 什么是面向无连接的？
    // 面向无连接的通信方式，不需要先建立连接，直接进行通信
    // protocol 是协议，0 表示默认协议
    // 默认协议是什么？
    // 默认协议是 TCP 协议
    // 还有什么协议？
    // 还有 UDP 协议，UDP 协议是用户数据报协议，是一种面向无连接的、不可靠的、基于数据报的传输层协议    
    // 返回值为 socket 描述符，如果返回值小于0，则表示创建失败  
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket creation failed");
        return 1;
    }   

    // setsockopt 函数是 POSIX 标准库中的一个函数，用于设置 socket 选项
    // 它的原型是：int setsockopt(int sockfd, int level, int optname, const void *optval, socklen_t optlen);
    // sockfd 是 socket 描述符，level 是选项级别，SOL_SOCKET 表示套接字选项
    // optname 是选项名称，SO_REUSEADDR 表示允许地址重用
    // optval 是选项值，&opt 表示 opt 的地址
    // optlen 是选项值的长度，sizeof(opt) 表示 opt 的长度
    // 为什么要设置socket选项？ 
    // 因为当服务器重启时，如果端口没有释放，可能会导致服务器无法正常启动
    // 设置socket选项，允许地址重用，可以避免这个问题   
    // level 可以选择 SOL_SOCKET, IPPROTO_TCP, IPPROTO_IP, IPPROTO_IPV6 等  
    // 为什么选择 SOL_SOCKET？
    // 因为 SO_REUSEADDR 是套接字选项，属于 SOL_SOCKET 级别 
    // 还有其他级别吗？
    // 是的，还有其他级别，比如 IPPROTO_TCP, IPPROTO_IP, IPPROTO_IPV6 等
    // 都是什么意思？
    // 为什么不选择 IPPROTO_TCP？
    // 因为 SO_REUSEADDR 是套接字选项，属于 SOL_SOCKET 级别 
    // 为什么opt = 1？
    // 因为 opt 是一个整数，1 表示 true，0 表示 false
    // 为什么opt = 1表示true？
    // 因为 SO_REUSEADDR 选项的值为 1 表示允许地址重用

 
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        return 1;
    }

    // 配置服务器地址
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;   // 设置地址族为 IPv4    
    server_addr.sin_addr.s_addr = INADDR_ANY;  // 监听所有网络接口
    server_addr.sin_port = htons(12345);       // 使用12345端口 
    // 可以使用多个端口吗？
    // 可以，但是需要使用多个socket，每个socket使用不同的端口
    // 为什么需要使用多个socket？
    // 因为一个socket只能绑定一个端口，如果需要使用多个端口，需要使用多个socket


    // 绑定socket到指定地址和端口
    if (bind(server_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind failed");
        return 1;
    }

    // 开始监听连接请求
    // listen 函数是 POSIX 标准库中的一个函数，用于监听连接请求
    // 它的原型是：int listen(int sockfd, int backlog);
    // sockfd 是 socket 描述符，backlog 是最大连接数
    // 如果只有 1 个连接，为什么需要设置最大连接数？    
    // 因为需要处理多个连接，所以需要设置最大连接数
    // 为什么是3？
    // 因为3是一个经验值，如果需要更多的连接数，可以修改这个值
    // 返回值为0表示成功，-1表示失败    
    // 这个是设置 socket 的监听队列长度，如果超过这个长度，新的连接会被拒绝
    // 设置成功返回0，失败返回-1    
    // 是不是只需要listen一次？
    // 是的，只需要listen一次
    // 为什么需要listen？
    // 因为需要监听连接请求，所以需要listen
    // 为什么需要监听连接请求？
    // 因为需要处理多个连接，所以需要监听连接请求 
    if (listen(server_fd, 3) < 0) {
        perror("listen failed");
        return 1;
    }

    std::cout << "[Server] Waiting for connections on port 12345...\n";

    while (true) {
        // 接受客户端连接
        // accept 函数是 POSIX 标准库中的一个函数，用于接受客户端连接
        // 它的原型是：int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        // sockfd 是 socket 描述符，addr 是客户端地址，addrlen 是客户端地址长度
        // 返回值为客户端socket描述符，如果返回值小于0，则表示接受失败
        // 为什么需要accept？
        // 因为需要接受客户端连接，所以需要accept   
        // 为什么这里初始化 client_addr 为空？
        // 因为需要接受客户端连接，所以需要初始化 client_addr 为空  
        // 接收完客户端连接后， client_addr 和 client_len 会被填充
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        int client_fd = accept(server_fd, (struct sockaddr*)&client_addr, &client_len);        
        if (client_fd < 0) {
            perror("accept failed");
            continue;
        }

        // 使用 getpeername 获取客户端信息
        // getpeername 函数是 POSIX 标准库中的一个函数，用于获取客户端信息
        // 它的原型是：int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
        // sockfd 是 socket 描述符，addr 是客户端地址，addrlen 是客户端地址长度
        // 返回值为0表示成功，-1表示失败
        // 为什么需要getpeername？
        // 因为需要获取客户端信息，所以需要getpeername  
        sockaddr_in peer_addr{};
        socklen_t peer_len = sizeof(peer_addr);
        if (getpeername(client_fd, (struct sockaddr*)&peer_addr, &peer_len) == 0) {
            char client_ip[INET_ADDRSTRLEN];
            // inet_ntop 函数是 POSIX 标准库中的一个函数，用于将网络地址转换为字符串
            // 它的原型是：const char *inet_ntop(int af, const void *src, char *dst, socklen_t size);
            // af 是地址族，AF_INET 表示 IPv4 地址族
            // src 是源地址，peer_addr.sin_addr 是客户端地址
            // dst 是目标地址，client_ip 是客户端地址字符串
            // size 是目标地址长度，INET_ADDRSTRLEN 是客户端地址字符串长度  
            inet_ntop(AF_INET, &peer_addr.sin_addr, client_ip, INET_ADDRSTRLEN);
            std::cout << "[Server] New connection from " << client_ip << ":" 
                      << ntohs(peer_addr.sin_port) << "\n";
        } else {
            perror("getpeername failed");
        }

        // 创建协议对象处理通信
        Protocol proto(client_fd);

        // 处理客户端请求
        while (true) {
            Packet request;
            if (!proto.receive_packet(request)) {
                std::cout << "[Server] Client disconnected.\n";
                break;
            }

            // 创建响应包
            // 为什么不加头部？
            // 因为不需要加头部，因为头部已经在 Packet 结构体中定义了   
            Packet response;
            if (request.payload.starts_with("#weather ")) {
                std::string city = request.payload.substr(9);  // 提取城市名
                response.payload = fetch_weather(city);        // 拉取天气信息
            } else {
                // 普通回显逻辑
                response.payload = "Server received: " + request.payload;
            }
            response.length = response.payload.length();
            // 计算并设置校验和
            response.checksum = calculate_checksum(std::vector<uint8_t>(response.payload.begin(), response.payload.end()));

            // 发送响应
            if (!proto.send_packet(response)) {
                std::cout << "[Server] Failed to send response.\n";
                break;
            }
        }

        // 关闭客户端连接
        close(client_fd);
    }

    // 关闭服务器socket
    close(server_fd);
    return 0;
}

