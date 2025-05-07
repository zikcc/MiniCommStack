// client.cpp
#include <arpa/inet.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

#include "net/Packet.hpp"
#include "net/Protocol.hpp"

int main(int argc, char* argv[]) {
    const char* server_ip = (argc > 1 ? argv[1] : "127.0.0.1");
    int server_port = (argc > 2 ? std::stoi(argv[2]) : 8888);

    // 1) 创建 TCP socket
    int sockfd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        perror("socket");
        return 1;
    }

    // 2) 连接到服务端
    sockaddr_in servaddr{};
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(server_port);
    if (::inet_pton(AF_INET, server_ip, &servaddr.sin_addr) != 1) {
        std::cerr << "Invalid IP: " << server_ip << "\n";
        ::close(sockfd);
        return 1;
    }
    if (::connect(sockfd, (sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("connect");
        ::close(sockfd);
        return 1;
    }
    std::cout << "Connected to " << server_ip << ":" << server_port << "\n";

    // 3) 用 Protocol 封装读写
    Protocol proto(sockfd);

    while (true) {
        // 4) 从 stdin 读一行
        std::cout << "Input> ";
        std::string line;
        if (!std::getline(std::cin, line) || line == "exit") {
            break;
        }

        // 5) 构造 Packet 并 enqueue 到 Protocol
        Packet pkt;
        pkt.header = 0xABCD;
        pkt.payload = line;
        pkt.length = static_cast<uint32_t>(line.size());
        pkt.checksum =
            calculate_checksum(std::vector<uint8_t>(line.begin(), line.end()));
        proto.enqueuePacket(pkt);

        // 6) flushSendBuffer 直到 send_buffer_ 清空
        int saved_errno = 0;
        while (proto.hasPendingSendData()) {
            if (!proto.flushSendBuffer(saved_errno)) {
                std::cerr << "send failed, errno=" << saved_errno << "\n";
                goto CLEANUP;
            }
            // 非阻塞下 EAGAIN/EWOULDBLOCK 正常重试
            if (saved_errno == EAGAIN || saved_errno == EWOULDBLOCK) {
                // 也可以加个 usleep() 限速，这里直接紧循环
                continue;
            }
        }

        // 7) 接收服务端响应包
        Packet resp;
        while (true) {
            auto status = proto.tryReceivePacket(resp);
            if (status == Protocol::ReadStatus::OK) {
                std::cout << "Echo> " << resp.payload << "\n\n";
                break;
            }
            if (status == Protocol::ReadStatus::Error) {
                std::cerr << "server closed or error\n";
                goto CLEANUP;
            }
            // NeedRetry：继续循环，从 socket 再读
        }
    }

CLEANUP:
    ::close(sockfd);
    return 0;
}
