#ifndef PACKET_HPP
#define PACKET_HPP

#include <string>
#include <vector>
#include <cstdint>  // 用于 uint32_t 这类类型   

// 计算校验和：将数据按字节累加
uint16_t calculate_checksum(const std::vector<uint8_t>& data);

/*
[header(2)][length(4)][payload(内容)][checksum(2)]
*/
struct Packet {
    uint16_t header = 0xABCD;     // 2 字节：固定魔数标志
    uint32_t length;              // 4 字节：payload长度
    std::string payload;          // n 字节：正文
    uint16_t checksum;            // 2 字节：校验值

    std::vector<uint8_t> serialize() const;
    /* const 成员函数，表示该函数不会修改对象的成员变量 */
    static Packet deserialize(const std::vector<uint8_t>& data);
    /* 静态成员函数, 它可以通过类名直接调用，而不需要创建 Packet 的实例。*/
};

#endif
