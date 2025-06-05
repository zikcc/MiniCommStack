// PacketB.cpp
#pragma once
#include "net/packet//PacketB.hpp"
/*
+------------+--------+------------+----------------+
| TypeID(2) | Ver(1) | Length(4B) | Payload(n字节) | 
+------------+--------+-----------+---------------+
*/

PacketB::PacketB(std::vector<uint8_t> p) : payload(std::move(p)) {}

uint16_t PacketB::getTypeID() const {
    return TYPE_ID;
}

std::vector<uint8_t> PacketB::serialize() const {
    std::vector<uint8_t> buffer;
    // 头部固定长度 = 2 + 1 + 4 + payload
    uint32_t length = static_cast<uint32_t>(payload.size());

    buffer.resize(2 + 1 + 4 + length);

    // TypeID 2B
    buffer[0] = (TYPE_ID >> 8) & 0xFF;
    buffer[1] = TYPE_ID & 0xFF;

    // Version 1B
    buffer[2] = VERSION;

    // Length 4B
    buffer[3] = (length >> 24) & 0xFF;
    buffer[4] = (length >> 16) & 0xFF;
    buffer[5] = (length >> 8) & 0xFF;
    buffer[6] = length & 0xFF;

    // Payload n字节
    std::memcpy(&buffer[7], payload.data(), length);

    return buffer;
}

// 反序列化，成功则返回PacketB智能指针，失败返回nullptr
std::shared_ptr<BasePacket> PacketB::deserialize(const std::vector<uint8_t>& raw) {
    if (raw.size() < 2+1+2+4+2) return nullptr; // 长度至少头部+校验
    // TypeID
    uint16_t TypeID = (raw[0] << 8) | raw[1];
    if (TypeID != TYPE_ID) return nullptr;

    // Version
    uint8_t ver = raw[2];
    if (ver != VERSION) return nullptr;

    // Length
    uint32_t length = (raw[5] << 24) | (raw[6] << 16) | (raw[7] << 8) | raw[8];
    if (raw.size() != 2+1+2+4 + length + 2) return nullptr; // 长度不符

    // Payload
    std::vector<uint8_t> payload(raw.begin() + 9, raw.begin() + 9 + length);

    return std::make_shared<PacketB>(std::move(payload));
}


