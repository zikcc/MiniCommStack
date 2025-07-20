// +-----------+---------+--------+--------+------------+-------------+
// | TypeID(2) | Flags(1)| Len(4) | IV(16) | Payload(n) | Checksum(2) |
// +-----------+---------+--------+--------+------------+-------------+

#include "net/packet/PacketSecure.hpp"
#include <cstdint>
#include <openssl/aes.h>
#include <openssl/rand.h>

PacketSecure::PacketSecure(std::string payload, 
                            bool compress, 
                            bool encrypt, uint8_t ver)
    : version(ver), raw_payload(std::move(payload)) {

    setFlag(SecurityFlags::COMPRESSED, compress);
    setFlag(SecurityFlags::ENCRYPTED, encrypt);

    if (encrypt) {
        iv.resize(16);
        RAND_bytes(iv.data(), iv.size());
    }
}

std::vector<uint8_t> PacketSecure::serialize() const {
    std::vector<uint8_t> buffer;

    // TypeID
    buffer.push_back((TYPE_ID >> 8) & 0xFF);
    buffer.push_back(TYPE_ID & 0xFF);

    // Flags
    buffer.push_back(flags);

    // 原始长度
    uint32_t raw_len = raw_payload.size();
    buffer.push_back((raw_len >> 24) & 0xFF);
    buffer.push_back((raw_len >> 16) & 0xFF);
    buffer.push_back((raw_len >> 8) & 0xFF);
    buffer.push_back((raw_len) & 0xFF);

    // IV
    buffer.insert(buffer.end(), iv.begin(), iv.end());

    // Payload：此处未真正实现压缩/加密逻辑
    std::vector<uint8_t> encoded_payload(raw_payload.begin(), raw_payload.end());
    buffer.insert(buffer.end(), encoded_payload.begin(), encoded_payload.end());

    // Checksum（简化实现）
    uint16_t checksum = 0xFFFF;
    buffer.push_back((checksum >> 8) & 0xFF);
    buffer.push_back(checksum & 0xFF);

    return buffer;
}
std::shared_ptr<IPacket> PacketSecure::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 2 + 1 + 4 + 16 + 2) return nullptr;

    PacketSecure pkt;

    size_t offset = 2;
    pkt.flags = data[offset++];

    uint32_t length = (data[offset] << 24) | (data[offset + 1] << 16) |
                      (data[offset + 2] << 8) | (data[offset + 3]);
    offset += 4;

    pkt.iv.assign(data.begin() + offset, data.begin() + offset + 16);
    offset += 16;

    size_t payload_size = data.size() - offset - 2;  // excluding checksum
    std::string payload(data.begin() + offset, data.begin() + offset + payload_size);
    pkt.raw_payload = std::move(payload);

    // 解压/解密略...

    return std::make_shared<PacketSecure>(pkt);
}
