// PacketB.hpp
#pragma once
#include "net/packet/BasePacket.hpp"
#include <cstring>  // memcpy
#include <memory>
/*
+------------+--------+------------+----------------+
| TypeID(2) | Ver(1) | Length(4B) | Payload(n字节) | 
+------------+--------+-----------+---------------+
*/
class PacketB : public BasePacket {
    public:
        static constexpr uint16_t TYPE_ID = 0x1002;
        static constexpr uint8_t VERSION = 2;
        std::vector<uint8_t> payload;
    
        PacketB() = default;
        explicit PacketB(std::vector<uint8_t> p);
    
        uint16_t getTypeID() const override;
        std::vector<uint8_t> serialize() const override;
        
        // 反序列化，成功则返回PacketA智能指针，失败返回nullptr
        static std::shared_ptr<BasePacket> deserialize(const std::vector<uint8_t>& raw);
};