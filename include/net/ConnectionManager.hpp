#pragma once  // 防止头文件重复包含

#include <memory>
#include <shared_mutex>
#include <unordered_map>

#include "net/Connection.hpp"

class ConnectionManager {
   public:
    void addConnection(int fd, std::shared_ptr<Connection> conn);
    void removeConnection(int fd);
    std::shared_ptr<Connection> getConnection(int fd) const;
    const std::unordered_map<int, std::shared_ptr<Connection>>&
    getAllConnections() const {
        return connections_;
    }

   private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<int, std::shared_ptr<Connection>> connections_;
};
