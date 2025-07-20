#include <cstdint>
#include <vector>
class BasePacket {
    public:
        virtual ~BasePacket() = default;
        virtual uint16_t getTypeID() const = 0;  // 必须由子类实现协议类型ID
        virtual uint8_t getVersion() const = 0;  // 协议版本控制
   
    
        virtual std::vector<uint8_t> serialize() const = 0;
    
        // 每种 Packet 自己定义其反序列化函数，不能放在 BasePacket 中
        // 不需要统一格式（无 magic、length、checksum 也可以）
        // 实现一个PacketFactory类，用于自动化检测data是哪个paket(根据前两位的typeid)
        // 每个packet的格式需要注册到工厂
        // PacketFactory::instance().registerType(PacketA::TYPE_ID, PacketA::deserialize);

};
    