// PacketA.hpp
#pragma once
#include "net/packet/BasePacket.hpp"
#include <cstring>  // memcpy
#include <memory>
/*
+------------+--------+-----------+------------+----------------+------------+
| TypeID(2) | Ver(1) | Magic(2B)  | Length(4B) | Payload(n字节) | Checksum(2B)|
+------------+--------+-----------+------------+----------------+------------+
*/
class PacketA : public BasePacket {
    public:
        static constexpr uint16_t TYPE_ID = 0x1001;
        static constexpr uint8_t VERSION = 1;
        static constexpr uint16_t MAGIC = 0xABCD;
    
        std::vector<uint8_t> payload;
    
        PacketA() = default;
        explicit PacketA(std::vector<uint8_t> p);
    
        uint16_t getTypeID() const override;
        std::vector<uint8_t> serialize() const override;
        
        // 反序列化，成功则返回PacketA智能指针，失败返回nullptr
        static std::shared_ptr<BasePacket> deserialize(const std::vector<uint8_t>& raw);
        static uint16_t calculateChecksum(const std::vector<uint8_t>& data);
};