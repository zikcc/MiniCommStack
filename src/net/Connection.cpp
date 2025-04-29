#include "net/Connection.hpp"
#include <errno.h>
#include <cstring>
#include <iostream>

constexpr size_t BUFFER_SIZE = 4096;

Connection::Connection(int fd) : fd(fd) {
    readBuffer.reserve(BUFFER_SIZE);
    writeBuffer.reserve(BUFFER_SIZE);
}

Connection::~Connection() {
    close(fd);
}

int Connection::getFd() const {
    return fd;
}

bool Connection::readToBuffer() {
    char buf[BUFFER_SIZE];
    while (true) {
        ssize_t n = recv(fd, buf, sizeof(buf), 0);
        if (n > 0) {
            readBuffer.insert(readBuffer.end(), buf, buf + n);
            parsePacketsFromBuffer();
        } else if (n == 0) {
            return false; // 连接关闭
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break; // 非阻塞无数据
            }
            return false; // 其他错误
        }
    }
    return true;
}

void Connection::parsePacketsFromBuffer() {
    while (true) {
        if (readBuffer.size() < Packet::header.size()) break;

        uint32_t packetSize = Packet::peekLength(readBuffer.data());
        if (readBuffer.size() < packetSize) break;

        std::shared_ptr<Packet> pkt = std::make_shared<Packet>();
        pkt->deserialize(readBuffer.data(), packetSize);
        receivedPackets.push(pkt);

        readBuffer.erase(readBuffer.begin(), readBuffer.begin() + packetSize);
    }
}

bool Connection::hasCompletePacket() const {
    return !receivedPackets.empty();
}

std::shared_ptr<Packet> Connection::getPacket() {
    if (receivedPackets.empty()) return nullptr;
    auto pkt = receivedPackets.front();
    receivedPackets.pop();
    return pkt;
}

void Connection::sendPacket(const Packet& packet) {
    std::vector<char> raw = packet.serialize();
    writeBuffer.insert(writeBuffer.end(), raw.begin(), raw.end());
}

bool Connection::writeFromBuffer() {
    while (!writeBuffer.empty()) {
        ssize_t n = send(fd, writeBuffer.data(), writeBuffer.size(), 0);
        if (n > 0) {
            writeBuffer.erase(writeBuffer.begin(), writeBuffer.begin() + n);
        } else {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            return false; // 错误
        }
    }
    return true;
}

bool Connection::hasPendingWrite() const {
    return !writeBuffer.empty();
}
