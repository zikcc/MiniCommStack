#include "net/packet/PacketRegistry.hpp"

PacketFactory& PacketFactory::instance() {
    static PacketFactory factory;
    return factory;
}

void PacketFactory::registerType(uint16_t typeID, Deserializer deserializer) {
    registry_[typeID] = std::move(deserializer);
}

std::shared_ptr<BasePacket> PacketFactory::tryDeserialize(const std::vector<uint8_t>& data) {
    if (data.size() < 2) return nullptr;

    uint16_t typeID = (data[0] << 8) | data[1];
    auto it = registry_.find(typeID);
    if (it != registry_.end()) {
        return it->second(data);  // 调用该类型的反序列化器
    }
    return nullptr;
}
