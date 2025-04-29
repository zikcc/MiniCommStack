#pragma once
#include <string>
#include <vector>
#include <queue>
#include <memory>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "Packet.hpp"  // 你自定义的数据包结构

class Connection {
public:
    // 构造函数 
    // explicit 禁止隐式转换
    // 禁止隐式转换，防止出现 Connection conn = 10; 这样的错误
    // 什么是隐式转换？
    // 隐式转换是指在某些情况下，编译器会自动将一种类型转换为另一种类型。
    // 例如，当一个函数需要一个 int 类型的参数，而你传递了一个 double 类型的值时，编译器会自动将 double 转换为 int。
    // 隐式转换可能会导致意外的行为，因此使用 explicit 关键字可以防止这种情况发生。
    explicit Connection(int fd);
    // 析构函数
    ~Connection();

    // 获取文件描述符   
    int getFd() const;

    // 非阻塞读入数据
    bool readToBuffer();

    // 非阻塞写出数据
    bool writeFromBuffer();

    // 检查是否有完整的数据包（基于 Packet 协议）
    bool hasCompletePacket() const;

    // 拿出一个完整数据包
    std::shared_ptr<Packet> getPacket();

    // 发送一个 Packet
    void sendPacket(const Packet& packet);

    // 是否还需要继续写（用于 epoll 的 EPOLLOUT）
    bool hasPendingWrite() const;

private:
    // 文件描述符
    int fd;

    // 读缓冲区
    std::vector<char> readBuffer;   // 拼包用
    std::vector<char> writeBuffer;  // 待写出数据
    // 为什么用shared_ptr？
    // 因为Packet对象可能被多个Connection共享，所以使用shared_ptr来管理Packet对象的生命周期 
    std::queue<std::shared_ptr<Packet>> receivedPackets;

    // 拼包内部使用
    void parsePacketsFromBuffer();
};
