# MiniCommStack

一个简单的C++ TCP通信示例项目，展示了基本的客户端-服务器通信架构。

## 项目结构

```
MiniCommStack/
├── include/
│   ├── Packet.hpp    # 数据包定义
│   └── Protocol.hpp  # 通信协议定义
├── src/
│   ├── Packet.cpp    # 数据包实现
│   ├── Protocol.cpp  # 通信协议实现
│   ├── Server.cpp    # 服务器程序
│   └── Client.cpp    # 客户端程序
└── CMakeLists.txt    # CMake构建配置
```

## 功能特点

- 基于TCP的可靠通信
- 自定义数据包格式，包含：
  - 2字节魔数标识（0xABCD）
  - 4字节数据长度
  - 可变长度数据负载
  - 2字节校验和
- 支持数据的序列化和反序列化
- 简单的错误处理机制

## 构建方法

1. 创建构建目录：
```bash
mkdir build
cd build
```

2. 配置项目：
```bash
cmake ..
```

3. 编译项目：
```bash
make
```

## 运行方法

1. 首先启动服务器：
```bash
./server
```

2. 在另一个终端启动客户端：
```bash
./client
```

3. 在客户端输入消息，服务器会返回确认消息。

## 通信协议说明

数据包格式：
```
+--------+--------+--------+--------+
| Header | Length | Payload| Checksum|
|  (2B)  |  (4B)  |  (nB)  |  (2B)  |
+--------+--------+--------+--------+
```

- Header: 固定值0xABCD，用于标识数据包
- Length: 数据负载的长度
- Payload: 实际数据内容
- Checksum: 校验和，用于验证数据完整性

## 注意事项

- 服务器默认监听12345端口
- 客户端默认连接到localhost:12345
- 确保防火墙允许该端口的通信

