#include "net/ConnectionManager.hpp"

void ConnectionManager::addConnection(int fd,
                                      std::shared_ptr<Connection> conn) {
    std::unique_lock lock(mutex_);
    connections_.emplace(fd, std::move(conn));
}

void ConnectionManager::removeConnection(int fd) {
    std::unique_lock lock(mutex_);
    connections_.erase(fd);
}

std::shared_ptr<Connection> ConnectionManager::getConnection(int fd) const {
    std::shared_lock lock(mutex_);
    auto it = connections_.find(fd);
    return (it != connections_.end()) ? it->second : nullptr;
}
