#include <memory>
#include <functional>
#include <vector>
#include "net/packet/BasePacket.hpp"

class PacketFactory {
    public:
        using Deserializer = std::function<std::shared_ptr<BasePacket>(const std::vector<uint8_t>&)>;

        static PacketFactory& instance();
        
        void registerType(uint16_t typeID, Deserializer deserializer);
        std::shared_ptr<BasePacket> tryDeserialize(const std::vector<uint8_t>& data);
    
    private:
        std::unordered_map<uint16_t, Deserializer> registry_;
};