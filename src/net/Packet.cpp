#include "net/Packet.hpp"
#include <cstring>
#include <stdexcept>
#include <arpa/inet.h>

// 计算校验和：将数据按字节累加
// 校验和用于验证数据在传输过程中是否被篡改
uint16_t calculate_checksum(const std::vector<uint8_t>& data) {
    uint16_t sum = 0;
    // 这里的 byte 是字符吗？
    // 不是，是 unsigned char 类型，表示一个字节
    // 这里累加是转为ascii码相加吗？
    // 不是，是按字节累加。每个字符在内存中的存储形式就是它的字节值，所以我们直接用字节值（即 uint8_t 类型的数字）进行累加。
    // 能举个例子吗？
    // 比如："123" 这个字符串，它的字节表示为：49 50 51，累加的结果是：49 + 50 + 51 = 150   
    // 为什么 "1" 的字节表示为 49？ 
    // 因为 "1" 的 ASCII 码是 49   
    // 汉字能用ASCII码表示吗？      
    // 不能，汉字是多字节字符，需要用 UTF-8 或 GBK 等编码方式表示
    // 如果 data 是汉字，那累加的结果是多少？
    // 比如："你好" 这个字符串，它的字节表示为：228 189 160 229 165 189，累加的结果是：228 + 189 + 160 + 229 + 165 + 189 = 1096 
    // "你好"的UTF-8编码为：E4 B8 AD E5 A5 BD，它的字节表示为：228 189 160 229 165 189，累加的结果是：228 + 189 + 160 + 229 + 165 + 189 = 1096 
    // 那返回值是1096吗？
    // 不是，返回值是 1096 的低 16 位，即 1096 % 65536 = 1096      
    // 为什么一会是 ascii 码，一会是 utf-8 编码？
    // 因为 ascii 码是单字节编码，而 utf-8 是多字节编码
    // 在进行 累加的时候会自动匹配编码方式吗？
    // 累加的时候，不会去判断编码方式。它只是按字节进行处理。因此，不管是 ASCII 还是 UTF-8，for (uint8_t byte : data) 都是按字节来进行操作的，不会自动识别字符的编码方式。
    // 对于字符串 "你好"，它的每个字符就是一个字节流（通过 UTF-8 编码），我们累加这些字节即可，不需要特别去匹配编码。
    // 那为什么 "1" 的累加结果是 49，而 "你好" 的累加结果是 1096？
    // 因为 "1" 的 ASCII 码是 49，而 "你好" 的 UTF-8 编码为：E4 B8 AD E5 A5 BD，它的字节表示为：228 189 160 229 165 189，累加的结果是：228 + 189 + 160 + 229 + 165 + 189 = 1096
    // 那为什么 "1" 的累加结果是 49，而 "你好" 的累加结果是 1096？
    // 因为 "1" 的 ASCII 码是 49，而 "你好" 的 UTF-8 编码为：E4 B8 AD E5 A5 BD，它的字节表示为：228 189 160 229 165 189，累加的结果是：228 + 189 + 160 + 229 + 165 + 189 = 1096

    
    for (uint8_t byte : data) {
        sum += byte;
    }
    return sum;
}

// 将Packet对象序列化为字节流
// 序列化是将数据结构转换为字节流的过程，以便通过网络传输
// const 的作用：
// 1. 表示该函数不会修改对象的成员变量
// 2. 使得该函数可以被 const 对象调用   
// 函数名前面存在 Packet:: 表示该函数是 Packet 类的成员函数 
// 为什么可以直接访问成员变量？
// 因为 serialize 是 const 成员函数，它不会修改对象的成员变量
// 所以可以直接访问成员变量 
// 去掉 const 后还能访问成员变量吗？
// 不能，因为非 const 成员函数可以修改成员变量，所以不能保证成员变量在函数调用期间不变  
std::vector<uint8_t> Packet::serialize() const {
    // 计算需要的总字节数：header(2) + length(4) + payload长度 + checksum(2)
    size_t total_size = 2 + 4 + payload.length() + 2;
    // 创建一个大小为 total_size 的向量，用于存储序列化后的数据 
    std::vector<uint8_t> result(total_size);
    
    // 写入header (2字节)
    // 使用网络字节序（大端序）以确保跨平台兼容性
    // htons 是什么意思？
    // htons 是主机字节序（host byte order）到网络字节序（network byte order）的转换函数
    // 主机字节序：小端序（little-endian）
    // 网络字节序：大端序（big-endian）
    // 为什么要转换字节序？
    // 因为不同的系统可能使用不同的字节序，为了避免在不同系统之间传输数据时出现兼容性问题，我们需要将数据转换为网络字节序
    // 为什么要使用大端序？
    // 因为大端序在网络传输中更常用，可以避免在不同系统之间传输数据时出现兼容性问题
    uint16_t network_header = htons(header);
    // 将 network_header 的值存储到 result 向量的第一个元素
    // 解释一下 memcpy 的用法   
    // memcpy 是 C 标准库中的一个函数，用于将一块内存的内容复制到另一块内存
    // 它的原型是：void *memcpy(void *dest, const void *src, size_t n);
    // dest 是目标内存的指针，src 是源内存的指针，n 是要复制的字节数
    // 这里将 network_header 的值存储到 result 向量的第一个元素 
    // result.data() 返回 result 向量的第一个元素的地址
    // &network_header 是 network_header 的地址
    // 2 是要复制的字节数
    memcpy(result.data(), &network_header, 2);
    // 写入length (4字节)
    // 为什么这里要使用 htonl 而不是 htons？
    // 因为 length 是 32 位的，而 htons 只能处理 16 位的数据
    // 所以需要使用 htonl 来处理 32 位的数据 
    uint32_t network_length = htonl(length);
    // 为什么 +2 而不是 +4？
    // 因为 result 向量的第一个元素已经占用了 2 个字节，所以从第二个元素开始存储    
    // result 里的元素是 uint8_t 类型，所以每个元素占 1 个字节
    // header 已经占用了 2 个字节，所以从第三个元素开始存储 
    memcpy(result.data() + 2, &network_length, 4);
    
    // 写入payload
    // 因为 header 已经占用了 2 个字节，length 已经占用了 4 个字节，所以从第六个元素开始存储
    // payload 是 std::string 类型，所以每个字符占 1 个字节
    // 为什么这里不进行大端序转换？
    // payload 是 UTF-8 编码的字符串，每个字符占 1 个字节，因此无需额外转换
    // 对于 UTF-8 编码的字符串，每个字符即使是汉字，也会按字节存储
    memcpy(result.data() + 6, payload.c_str(), payload.length());
    
    // 计算并写入checksum (2字节)
    // checksum 为什么没调用 calculate_checksum？
    uint16_t network_checksum = htons(checksum);
  
    // 因为 checksum 占用了 2 个字节，所以从 total_size - 2 开始存储
    memcpy(result.data() + total_size - 2, &network_checksum, 2);
    
    return result;
}

// 从字节流反序列化为Packet结构体
// 反序列化是将字节流转换回数据结构的过程
Packet Packet::deserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 8) { // 最小长度：header(2) + length(4) + checksum(2)
        throw std::runtime_error("Invalid packet data: too short");
    }
    // Packet 是一个结构体
    Packet pkt{};
    
    // 读取header (2字节)
    uint16_t network_header;
    memcpy(&network_header, data.data(), 2);
    // ntohs 是什么意思？
    // ntohs 是网络字节序（network byte order）到主机字节序（host byte order）的转换函数
    // 为什么要转换字节序？
    // 因为网络字节序是大端序，而主机字节序是小端序，所以需要转换字节序 
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
    if (data.size() != pkt.length + 8) { // 8 = header(2) + length(4) + checksum(2)
        throw std::runtime_error("Invalid packet length");
    }
    
    // 读取payload
    // reinterpret_cast 是什么意思？
    // reinterpret_cast 是 C++ 标准库中的一个函数，用于将一个指针或引用转换为另一个指针或引用
    // 它的原型是：void *reinterpret_cast(void *expr);
    // expr 是要转换的指针或引用
    // 这里将 data.data() + 6 转换为 const char* 类型
    // 然后创建一个 std::string 对象，它的内容是 data.data() + 6 开始的 pkt.length 个字节   
    pkt.payload = std::string(
        reinterpret_cast<const char*>(data.data() + 6),
        pkt.length
    );
    
    // 读取checksum (2字节)
    uint16_t network_checksum;
    memcpy(&network_checksum, data.data() + data.size() - 2, 2);
    pkt.checksum = ntohs(network_checksum);
    
    // 验证校验和
    uint16_t calculated_checksum = calculate_checksum(
        std::vector<uint8_t>(data.begin() + 6, data.end() - 2)
    );
    if (pkt.checksum != calculated_checksum) {
        throw std::runtime_error("Checksum verification failed");
    }
    
    return pkt;
}


