#ifndef CONNECTION_H
#define CONNECTION_H

#include <string>
#include <unistd.h>
#include <vector>
#include "net/Protocol.hpp"
#include "net/Packet.hpp"

class Connection {
public:
    explicit Connection(int fd);
    ~Connection();

    int getFd() const;
    bool handleRead();  // 处理可读事件
    bool handleWrite(); // 可写时调用（可选）

private:
    int fd;
    Protocol proto;
    std::vector<Packet> pending_packets;  // 待发送的数据包
};

#endif // CONNECTION_H
