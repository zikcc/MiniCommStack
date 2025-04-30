#ifndef CONNECTION_MANAGER_H
#define CONNECTION_MANAGER_H

#include <unordered_map>
#include <memory>
#include "net/Connection.hpp"

class ConnectionManager {
public:
    void addConnection(int fd, std::shared_ptr<Connection> conn);
    void removeConnection(int fd);
    std::shared_ptr<Connection> getConnection(int fd);
    const std::unordered_map<int, std::shared_ptr<Connection>>& getAllConnections() const { return connections; }

private:
    std::unordered_map<int, std::shared_ptr<Connection>> connections;
};

#endif // CONNECTION_MANAGER_H
