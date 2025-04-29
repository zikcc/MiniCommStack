#pragma once

#include "net/Packet.hpp"
#include <string>

class Protocol {
public:
    // 构造函数 
    // 参数：socket_fd 是 socket 描述符 
    Protocol(int socket_fd);
    // 析构函数
    ~Protocol();
    // 发送Packet
    // 参数：pkt 是要发送的 Packet 对象
    bool send_packet(const Packet& pkt);
    // 接收Packet
    // 参数：pkt 是要接收的 Packet 对象
    // 返回值：true 表示接收成功，false 表示接收失败
    bool receive_packet(Packet& pkt);

private:
    // socket 描述符    
    int sockfd;
};
