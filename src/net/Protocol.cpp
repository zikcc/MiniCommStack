#include "net/Protocol.hpp"
#include <unistd.h>
#include <vector>
#include <iostream>
#include <cstring>
#include <arpa/inet.h>
// 实现 Protocol 的构造函数
// 参数：fd 是 socket 描述符
// 作用：初始化 socket 描述符
// sockfd(fd) 成员函数初始化列表的方式
Protocol::Protocol(int fd) : sockfd(fd) {}
// 实现析构函数
// 为什么这里可以拿到 sockfd？
// 因为 sockfd 是 Protocol 的成员变量，所以可以在析构函数中访问 
Protocol::~Protocol() {
    close(sockfd);
}
// 实现 send_packet 函数
bool Protocol::send_packet(const Packet& pkt) {
    auto data = pkt.serialize();
    size_t total = 0;
    while (total < data.size()) {
        // 写入数据到 socket
        // write 函数是 POSIX 标准库中的一个函数，用于将数据写入到文件描述符中
        // 它的原型是：ssize_t write(int fd, const void *buf, size_t count);
        // fd 是文件描述符，buf 是要写入的数据，count 是要写入的数据的长度
        // 返回为写入的字节数，如果返回值小于0，则表示写入失败  
        // 这里将 data.data() + total 转换为 const void* 类型
        // 然后写入到 sockfd 中，写入的长度是 data.size() - total   
        // 为什么要 + total 呢？
        // 因为 data.data() 是一个指针，指向 data 的第一个元素
        // 所以 data.data() + total 是指向 data 的第 total 个元素
        // 所以写入的长度是 data.size() - total 
        // 每次写入一个字节，sent 为1
        ssize_t sent = write(sockfd, data.data() + total, data.size() - total);
        if (sent <= 0) return false;
        total += sent;
    }
    return true;
}

// 接收一个完整的 Packet 数据包
// 数据包格式为：[header(2)][length(4)][payload(变长)][checksum(2)]
bool Protocol::receive_packet(Packet& pkt) {
    uint8_t len_buf[6];
    size_t total = 0;

    // 读取前6个字节：header(2) + length(4)
    // 为什么读取6个字节？
    // 因为 Packet 的前缀结构包含 header 和 length，分别是 2 和 4 字节
    while (total < 6) {
        ssize_t received = read(sockfd, len_buf + total, 6 - total);
        if (received <= 0) return false;  // 连接中断或发生错误
        total += received;
    }

    // 从 len_buf 中解析出 length 字段
    uint32_t network_length;
    memcpy(&network_length, len_buf + 2, 4);  // 后4字节是 length
    uint32_t payload_length = ntohl(network_length);  // 转为主机字节序

    // 计算剩余需要接收的字节数：payload长度 + checksum(2)
    size_t remaining = payload_length + 2;
    std::vector<uint8_t> buffer(remaining);

    total = 0;
    // 为什么要循环读取？
    //  因为 read() 并不保证一次就读完你请求的全部数据！
    //  read(sockfd, buf, n) 的语义是“最多读取 n 个字节”，但实际返回的字节数可能 远小于 n，甚至是 1 字节。
    while (total < remaining) {
        ssize_t received = read(sockfd, buffer.data() + total, remaining - total);
        if (received <= 0) return false;  // 接收失败
        total += received;
    }

    // 拼接完整数据：[header(2)][length(4)][payload][checksum(2)]
    std::vector<uint8_t> combined;
    // insert 函数是 C++ 标准库中的一个函数，用于将一个元素插入到容器中
    // first, last 是迭代器，表示要插入的元素的范围
    // 这里将 len_buf 插入到 combined 的末尾
    // 然后插入 buffer 的 begin 到 end 之间的元素
    combined.insert(combined.end(), len_buf, len_buf + 6);          // 插入 header+length
    combined.insert(combined.end(), buffer.begin(), buffer.end());  // 插入 payload + checksum

    // 调用反序列化函数
    pkt = Packet::deserialize(combined);
    return true;
}
