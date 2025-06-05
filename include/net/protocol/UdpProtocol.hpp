#pragma once
#include "net/protocol/BaseProtocol.hpp"
#include <vector>
#include <netinet/in.h>

class UdpProtocol : public BaseProtocol {
    public:
        UdpProtocol(int socket_fd, sockaddr_in peer_addr);
        ~UdpProtocol();

        ReadStatus tryReceivePacket(BasePacket& pkt) override;
        void enqueuePacket(const BasePacket& pkt) override;
        bool flushSendBuffer(int& saved_errno) override;
        bool hasPendingSendData() const override;

    private:
        const int sockfd_;
        sockaddr_in peer_;
        std::vector<uint8_t> send_buffer_;
};