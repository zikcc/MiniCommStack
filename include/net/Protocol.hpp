#pragma once  // 防止头文件重复包含
#include "Packet.hpp"
#include <vector>

class Protocol {
public:
    /**
     * 构造函数
     * @param socket_fd 已连接的 socket 文件描述符，由外部管理生命周期
     */
    explicit Protocol(int socket_fd);

    // 禁止拷贝构造和赋值
    Protocol(const Protocol&) = delete;
    Protocol& operator=(const Protocol&) = delete;

    ~Protocol();

    bool tryReceivePacket(Packet& pkt);
    void enqueuePacket(const Packet& pkt);
    bool flushSendBuffer();

private:
    bool parseFromBuffer(std::vector<uint8_t>& buffer, Packet& pkt);

    const int sockfd_;           
    std::vector<uint8_t> send_buffer_;
    std::vector<uint8_t> recv_buffer_;
};