#include "net/Packet.hpp"

#include <arpa/inet.h>

#include <cstring>
#include <stdexcept>

// 计算校验和：将数据按字节累加
// 校验和用于验证数据在传输过程中是否被篡改
uint16_t calculate_checksum(const std::vector<uint8_t>& data) {
    uint16_t sum = 0;
    for (uint8_t byte : data) {
        sum += byte;
    }
    return sum;
}

std::vector<uint8_t> Packet::serialize() const {
    // 计算需要的总字节数：header(2) + length(4) + payload长度 + checksum(2)
    size_t total_size = 2 + 4 + payload.length() + 2;
    // 创建一个大小为 total_size 的向量，用于存储序列化后的数据
    std::vector<uint8_t> result(total_size);

    uint16_t network_header = htons(header);
    memcpy(result.data(), &network_header, 2);
    uint32_t network_length = htonl(length);
    memcpy(result.data() + 2, &network_length, 4);
    memcpy(result.data() + 6, payload.c_str(), payload.length());
    uint16_t network_checksum = htons(checksum);
    memcpy(result.data() + total_size - 2, &network_checksum, 2);

    return result;
}

// 从字节流反序列化为Packet结构体
// 反序列化是将字节流转换回数据结构的过程
Packet Packet::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 8) {  // 最小长度：header(2) + length(4) + checksum(2)
        throw std::runtime_error("Invalid packet data: too short");
    }
    // Packet 是一个结构体
    Packet pkt{};

    // 读取header (2字节)
    uint16_t network_header;
    memcpy(&network_header, data.data(), 2);

    pkt.header = ntohs(network_header);

    // 验证header
    if (pkt.header != 0xABCD) {
        throw std::runtime_error("Invalid packet header");
    }

    // 读取length (4字节)
    uint32_t network_length;
    memcpy(&network_length, data.data() + 2, 4);
    pkt.length = ntohl(network_length);

    // 验证数据长度
    if (data.size() !=
        pkt.length + 8) {  // 8 = header(2) + length(4) + checksum(2)
        throw std::runtime_error("Invalid packet length");
    }

    pkt.payload =
        std::string(reinterpret_cast<const char*>(data.data() + 6), pkt.length);

    // 读取checksum (2字节)
    uint16_t network_checksum;
    memcpy(&network_checksum, data.data() + data.size() - 2, 2);
    pkt.checksum = ntohs(network_checksum);

    // 验证校验和
    uint16_t calculated_checksum = calculate_checksum(
        std::vector<uint8_t>(data.begin() + 6, data.end() - 2));
    if (pkt.checksum != calculated_checksum) {
        throw std::runtime_error("Checksum verification failed");
    }

    return pkt;
}
