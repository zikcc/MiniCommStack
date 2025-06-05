#pragma once
#include "net/packet/BasePacket.hpp"

class BaseProtocol {
    public:
        enum class ReadStatus {OK, NeedRetry, Error};

        virtual ~BaseProtocol() = default;

        virtual ReadStatus tryReceivePacket(BasePacket& pkt) = 0;
        virtual void enqueuePacket(const BasePacket& pkt) = 0;
        virtual bool flushSendBuffer(int& saved_errno) = 0;
        virtual bool hasPendingSendData() const = 0;
};