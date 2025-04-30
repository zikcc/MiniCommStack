// src/Client.cpp
#include "net/Packet.hpp"
#include "net/Protocol.hpp"
#include <netinet/in.h>  // 包含网络地址结构体定义
#include <arpa/inet.h>   // 包含IP地址转换函数
#include <unistd.h>      // 包含POSIX操作系统API，如close()
#include <iostream>      // 用于标准输入输出
#include <string>        // 用于std::string

// 客户端程序的入口
int main() {
    // 创建一个TCP socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("socket");  // 输出错误信息
        return 1;          // 返回非零值表示错误
    }

    // 配置服务器地址
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;  // 使用IPv4地址
    server_addr.sin_port = htons(12345); // 将端口号转换为网络字节序
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // 将IP地址转换为网络字节序

    // 连接到服务器
    if (connect(sock_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");  // 输出错误信息
        return 1;           // 返回非零值表示错误
    }

    std::cout << "[Client] Connected to server.\n";
    Protocol proto(sock_fd);  // 创建Protocol对象，用于处理通信

    std::string input;
    while (true) {
        std::cout << "[Client] Enter message (or 'exit'): ";
        std::getline(std::cin, input);  // 从标准输入读取一行
        if (input == "exit") break;     // 如果输入"exit"，则退出循环

        // 创建数据包并发送
        Packet pkt;
        pkt.payload = input;  // 设置数据包的负载
        pkt.length = input.size();  // 设置数据包的长度
        pkt.checksum = calculate_checksum(std::vector<uint8_t>(input.begin(), input.end())); // 计算校验和
        proto.send_packet(pkt);  // 发送数据包

        // 接收服务器响应
        Packet response;
        if (proto.receive_packet(response)) {
            std::cout << "[Client] Server responded: " << response.payload << "\n";
        } else {
            std::cout << "[Client] Server closed connection.\n";
            break;
        }
    }

    // 关闭socket
    close(sock_fd);
    return 0;
}
