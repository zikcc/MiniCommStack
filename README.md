# MiniCommStack

一个简单的 C++ 网络编程学习项目，用于学习和实践基本的网络通信原理。

## 项目概述

MiniCommStack 是一个面向学习的 C++ 网络编程项目，主要目的是通过实践来理解网络通信的基本原理。项目包含以下核心功能：

- 基于 TCP 的客户端-服务器通信
- 简单的数据包格式设计
- 基本的线程池实现
- 简单的日志系统
- 连接管理

## 项目结构

```
MiniCommStack/
├── include/                # 头文件目录
│   ├── app/               # 应用程序相关
│   │   ├── Server.hpp     # 服务器实现
│   │   └── Client.hpp     # 客户端实现
│   ├── net/               # 网络通信相关
│   │   ├── Packet.hpp     # 数据包定义
│   │   ├── Protocol.hpp   # 通信协议
│   │   └── Connection.hpp # 连接管理
│   ├── threading/         # 线程相关
│   │   └── ThreadPool.hpp # 线程池实现
│   └── utils/             # 工具类
│       ├── Logger.hpp     # 日志系统
│       └── Metrics.hpp    # 性能监控
├── src/                   # 源文件目录
│   ├── app/              # 应用程序实现
│   ├── net/              # 网络通信实现
│   ├── threading/        # 线程实现
│   └── utils/            # 工具类实现
├── main/                 # 主程序目录
│   └── main.cpp         # 程序入口
├── tests/                # 测试目录
├── CMakeLists.txt       # CMake 构建配置
└── LICENSE              # 许可证文件
```

## 功能特性

### 网络通信
- 基于 TCP 的客户端-服务器模型
- 简单的数据包格式：
  ```
  +--------+--------+--------+--------+
  | Header | Length | Payload| Checksum|
  |  (2B)  |  (4B)  |  (nB)  |  (2B)  |
  +--------+--------+--------+--------+
  ```
- 支持数据的序列化和反序列化
- 基本的错误处理机制

### 线程池
- 固定大小的线程池
- 任务队列管理
- 简单的任务调度

### 工具类
- 简单的日志系统（支持多级别日志）
- 基本的性能监控
- 连接管理

## 依赖项

- C++20 或更高版本
- CMake 3.10 或更高版本

## 构建方法

1. 安装依赖：
```bash
# Ubuntu/Debian
sudo apt-get update
sudo apt-get install cmake

# CentOS/RHEL
sudo yum install cmake
```

2. 构建项目：
```bash
mkdir build
cd build
cmake ..
make
```

## 运行方法

### 服务器
```bash
./server [options]
```

选项：
- `-p, --port`: 指定端口号（默认：8888）
- `-t, --threads`: 指定线程池大小（默认：CPU核心数）
- `-l, --log-level`: 指定日志级别（DEBUG/INFO/WARNING/ERROR/FATAL）

### 客户端
```bash
./client [options]
```

选项：
- `-h, --host`: 指定服务器地址（默认：localhost）
- `-p, --port`: 指定端口号（默认：8888）

## 学习要点

1. 网络编程基础
   - TCP 套接字编程
   - 客户端-服务器模型
   - 数据包设计
   - 错误处理

2. 多线程编程
   - 线程池设计
   - 任务队列
   - 线程同步
   - 资源管理

3. 工具类设计
   - 单例模式
   - 日志系统
   - 性能监控
   - 连接管理

## 开发计划

### 近期计划
- [ ] 完善错误处理机制
- [ ] 添加更多的单元测试
- [ ] 优化线程池实现
- [ ] 改进日志系统

### 长期计划
- [ ] 支持更多的图像处理算法
- [ ] 实现分布式爬虫
- [ ] 添加 Web 管理界面
- [ ] 支持更多的通信协议

## 贡献指南

1. Fork 项目
2. 创建特性分支 (`git checkout -b feature/AmazingFeature`)
3. 提交更改 (`git commit -m 'Add some AmazingFeature'`)
4. 推送到分支 (`git push origin feature/AmazingFeature`)
5. 创建 Pull Request

## 许可证

本项目采用 MIT 许可证 - 详见 [LICENSE](LICENSE) 文件


