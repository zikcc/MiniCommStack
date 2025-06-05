#pragma once  // 防止头文件重复包含
#include <vector>

#include "net/protocol/BaseProtocol.hpp"

class TcpProtocol : public BaseProtocol {
   public:
    explicit TcpProtocol(int socket_fd);

    // 禁止拷贝构造和赋值
    TcpProtocol(const TcpProtocol&) = delete;
    TcpProtocol& operator=(const TcpProtocol&) = delete;

    ~TcpProtocol();

    // enum class ReadStatus { OK, NeedRetry, Error };

    ReadStatus tryReceivePacket(BasePacket& pkt) override;
    void enqueuePacket(const BasePacket& pkt) override;
    bool flushSendBuffer(int& saved_errno) override;
    // 如果还有没发完的数据，返回 true
    bool hasPendingSendData() const override { return !send_buffer_.empty(); }

   private:
    bool parseFromBuffer(std::vector<uint8_t>& buffer, BasePacket& pkt);

    const int sockfd_;
    std::vector<uint8_t> send_buffer_;
    std::vector<uint8_t> recv_buffer_;
};