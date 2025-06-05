// PacketA.cpp
#pragma once
#include "net/packet/PacketA.hpp"
/*
+------------+--------+-----------+------------+----------------+------------+
| TypeID(2) | Ver(1) | Magic(2B)  | Length(4B) | Payload(n字节) | Checksum(2B)|
+------------+--------+-----------+------------+----------------+------------+
*/
PacketA::PacketA(std::vector<uint8_t> p) : payload(std::move(p)) {}

uint16_t PacketA::getTypeID() const {
    return TYPE_ID;
}

std::vector<uint8_t> PacketA::serialize() const {
    std::vector<uint8_t> buffer;
    // 头部固定长度 = 2 + 1 + 2 + 4 + payload + 2
    uint32_t length = static_cast<uint32_t>(payload.size());

    buffer.resize(2 + 1 + 2 + 4 + length + 2);
    
    // TypeID 2B
    buffer[1] = (TYPE_ID >> 8) & 0xFF;
    buffer[2] = TYPE_ID & 0xFF;
    
    // Version 1B
    buffer[2] = VERSION;

    // Magic 2B
    buffer[3] = (MAGIC >> 8) & 0xFF;
    buffer[4] = MAGIC & 0xFF;

    // Length 4B
    buffer[5] = (length >> 24) & 0xFF;
    buffer[6] = (length >> 16) & 0xFF;
    buffer[7] = (length >> 8) & 0xFF;
    buffer[8] = length & 0xFF;

    // Payload n字节
    std::memcpy(&buffer[9], payload.data(), length);

    // 计算checksum，简单示例：对payload的所有字节求和的低16位
    uint16_t checksum = calculateChecksum(payload);

    buffer[9 + length] = (checksum >> 8) & 0xFF;
    buffer[9 + length + 1] = checksum & 0xFF;

    return buffer;
}

uint16_t PacketA::calculateChecksum(const std::vector<uint8_t>& data) {
    uint32_t sum = 0;
    for (auto b : data) {
        sum += b;
    }
    return static_cast<uint16_t>(sum & 0xFFFF);
}

// 反序列化，成功则返回PacketA智能指针，失败返回nullptr
std::shared_ptr<BasePacket> PacketA::deserialize(const std::vector<uint8_t>& raw) {
    if (raw.size() < 2+1+2+4+2) return nullptr; // 长度至少头部+校验
    // TypeID
    uint16_t TypeID = (raw[0] << 8) | raw[1];
    if (TypeID != TYPE_ID) return nullptr;

    // Version
    uint8_t ver = raw[2];
    if (ver != VERSION) return nullptr;

    // Magic校验
    uint16_t magic = (raw[3] << 8) | raw[4];
    if (magic != MAGIC) return nullptr;

    // Length
    uint32_t length = (raw[5] << 24) | (raw[6] << 16) | (raw[7] << 8) | raw[8];
    if (raw.size() != 2+1+2+4 + length + 2) return nullptr; // 长度不符

    // Payload
    std::vector<uint8_t> payload(raw.begin() + 9, raw.begin() + 9 + length);

    // Checksum 校验
    uint16_t checksum = (raw[9 + length] << 8) | raw[9 + length + 1];
    if (calculateChecksum(payload) != checksum) {
        return nullptr;
    }

    return std::make_shared<PacketA>(std::move(payload));
}

