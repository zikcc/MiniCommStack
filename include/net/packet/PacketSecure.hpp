#pragma once
#include "net/packet/BasePacket.hpp"
#include <string>
#include <memory>
/*
+-----------+---------+--------+--------+------------+-------------+
| TypeID(2) | Flags(1)| Len(4) | IV(16) | Payload(n) | Checksum(2) |
+-----------+---------+--------+--------+------------+-------------+
TypeID (2 字节)：固定格式，用于识别 packet 类型。
Flags (1 字节)：
    Bit 0: 是否压缩
    Bit 1: 是否加密
Len (4 字节)：原始 payload 长度（未压缩、未加密的长度）
IV (16 字节)：加密使用的初始化向量（例如用于 AES）
Payload (n 字节)：经过压缩/加密后的数据
Checksum (2 字节)：用于校验（可用 CRC 或简单校验）
*/
class PacketSecure : public BasePacket {
    public:
        static constexpr uint16_t TYPE_ID = 0x2001;
    
        uint8_t flags = 0;
        std::vector<uint8_t> iv;
        std::string raw_payload;  // 原始 payload（未压缩、未加密）
    
        PacketSecure() = default;
        explicit PacketSecure(std::string payload, bool compress = false, bool encrypt = false);
    
        uint16_t getTypeID() const override { return TYPE_ID; }
    
        std::vector<uint8_t> serialize() const override;
    
        static std::shared_ptr<BasePacket> deserialize(const std::vector<uint8_t>& data);
};
    