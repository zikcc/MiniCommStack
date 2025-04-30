#include "net/ConnectionManager.hpp"

void ConnectionManager::addConnection(int fd, std::shared_ptr<Connection> conn) {
    connections[fd] = conn;
}

void ConnectionManager::removeConnection(int fd) {
    connections.erase(fd);
}

std::shared_ptr<Connection> ConnectionManager::getConnection(int fd) {
    auto it = connections.find(fd);
    if (it != connections.end()) {
        return it->second;
    }
    return nullptr;
}
