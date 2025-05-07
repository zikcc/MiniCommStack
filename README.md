# MiniCommStack

*一个基于 epoll + 线程池的轻量级 TCP 协议栈示例，面向嵌入式 Linux 网络编程学习与演示。*

> **声明**
> 本项目为个人学习性质，实现了基本的高并发 TCP 服务器与简单协议，已做过压力测试，但尚未做全面安全加固与生产级优化。欢迎反馈、Issue 和 PR！

---

## 🎯 项目概述

* **语言标准**：C++20
* **构建系统**：CMake ≥ 3.10（支持 out‑of‑source 构建）
* **平台依赖**：Linux（`epoll`、非阻塞 socket）
* **并发模型**：`epoll` 边缘触发（`EPOLLET`）+ 自定义线程池
* **协议格式**：自定义 `Packet`（2 字节头 + 4 字节长度 + payload + 2 字节校验和）
* **压测工具**：`load_test` 使用多线程模拟客户端连接

---

## 🔧 关键组件说明

| 组件            | 功能描述                                                                                                                                                  |
|---------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------|
| **Server**    | - 初始化非阻塞 TCP 监听 socket，设置端口复用 & keep‑alive<br>- 创建 epoll 实例并将监听 socket 注册至事件循环<br>- 主循环中调用 `epoll_wait`：<br>  1. 新连接 → `accept4` → 注册客户端 fd（`EPOLLIN|EPOLLET`）<br>  2. 客户端事件 → 派发至线程池执行 `handleRead`/`handleWrite`<br>- 提供 `stop()`，优雅关闭线程池、清理所有连接、释放资源 |
| **Connection**| - 持有单个客户端的 fd、读写缓冲与协议上下文<br>- `handleRead()`：循环从 recv 缓冲区解析完整包 → 业务处理 → `enqueuePacket` 将响应入发送缓冲<br>- `handleWrite()`：尽量 flush 发送缓冲至 socket → 缓冲空后移除 `EPOLLOUT` 监听<br>- `modifyEpollEvents()` 封装动态增/删 `EPOLLOUT`                                          |
| **Protocol**  | - 管理发送 & 接收缓冲区 (`send_buffer_`, `recv_buffer_`)<br>- `tryReceivePacket()`：非阻塞解析已有数据；不足时 `recv()` 新数据后重试；返回 `OK`/`NeedRetry`/`Error`<br>- `enqueuePacket()`：序列化 `Packet` 并追加至发送缓冲区<br>- `flushSendBuffer()`：非阻塞 `send()` 缓冲区数据；报告部分发送或错误             |
| **Packet**    | - 自定义帧格式：<br>  1. 固定 2B 协议头<br>  2. 2B payload 长度<br>  3. 2B 校验和<br>  4. N B payload<br>- `serialize()`/`deserialize()` 实现网络字节序转换 + 校验和验证，解决 TCP 粘包/拆包                               |
| **ThreadPool**| - 固定数量工作线程，维护线程安全的任务队列<br>- `enqueue()` 提交 `std::function<void()>` 任务<br>- `shutdown()` 停止接收新任务，`wait()` 等待所有线程退出                                                   |
| **LoadTester**| - 客户端压测工具：批量创建非阻塞 socket，发起 `connect()`（`EINPROGRESS`）<br>- 每个 socket 用 epoll 监控 `EPOLLOUT`（连接完成）和 `EPOLLIN`（回显）<br>- 自动统计成功/失败并计算吞吐率                                                         |
| **main_server**| - 读取并校验命令行或配置参数（端口、线程池大小等）<br>- 注册 SIGINT/SIGTERM 信号处理回调，支持 Ctrl+C 优雅退出<br>- 调用 `Server::setup()` 初始化，`Server::run()` 进入事件循环                                   |
| **main_client**| - 交互式示例：从 stdin 读取用户输入并构造 `Packet`，通过 `Protocol` 发送至服务器并打印响应<br>- 演示协议层的同步读写用法                                                                             |

---

## 📚 背景知识与协议说明

### 1. I/O 多路复用与 epoll  
- **select/poll vs epoll**：`select`/`poll` 每次调用需线性扫描所有 fd (O(N))；`epoll` 内核维护就绪列表，用户态触发回调 (O(1))，性能更优。  
- **边缘触发 (EPOLLET)**：只在状态变化时通知，需一次性读写干净，否则不会再次触发；减少系统调用，但需处理更复杂。  
- **线程安全**：多线程对同一 epoll 实例进行 `epoll_ctl`/`epoll_wait` 安全，内核已内部协调。

### 2. Reactor 模式与 Proactor  
- **Reactor**：主线程负责 I/O 通知 (`epoll_wait`)，事件到来后分发给处理函数。  
- **Proactor**：操作系统完成异步 I/O 后通知应用，应用只需处理结果。  
- **本项目**：主线程做 Reactor 分发，读写逻辑在工作线程（Proactor 风格）异步执行，结合两者优势。

### 3. 自定义帧协议设计  
- **固定头 (Magic)**：简易协议标识，快速过滤非本协议流量。  
- **长度字段**：明确 payload 大小，解决粘包/拆包。  
- **校验和**：简易完整性校验（如 16 位加和），防止数据中途损坏。  
- **Payload**：字符或二进制均可，长度动态扩展。

### 4. 部分读写与缓存  
- `recv()`/`read()` 可能读取不到一个完整帧，需要累积在 `recv_buffer_` 中；  
- `send()`/`write()` 可能写入不完整，需要保留 `send_buffer_` 中的剩余数据，等待 `EPOLLOUT` 再续写。

### 5. 性能优化要点  
- **非阻塞 I/O**：避免单个 socket 操作阻塞调用线程。  
- **动态写事件**：只在有数据待发时注册 `EPOLLOUT`，发送完立即移除，减少空唤醒。  
- **线程池背压**：可限制队列长度，防止任务过多导致内存或 CPU 饱和。  
- **TCP Keep‑alive**：自动探测死连，释放无效资源。  
- **系统调优**：调整 `ulimit -n`、`net.ipv4.ip_local_port_range`、`tcp_tw_reuse` 等参数，以支撑大并发。


## 📁 目录结构

```
MiniCommStack
├──  include/                
├──  app/                  
├──     Server.hpp        
├──     ServerConfig.hpp       
├──  load_test/           
├──     LoadTester.hpp   
├──  net/                  
├──     Connection.hpp    
├──     ConnectionManager.hpp    
├──     Packet.hpp        
├──     Protocol.hpp      
├──  threading/           
├──     ThreadPool.hpp  
├── main/                 
├──  main_server.cpp  
├──  main_load_test.cpp
├──  main_client.cpp  
├── src/                    
├──  app/                
├──     Server.cpp      
├──  load_test/         
├──     LoadTester.cpp
├──  net/                
├──     Connection.cpp  
├──     ConnectionManager.cpp  
├──     Packet.cpp        
├──     Protocol.cpp      
├──  threading/        
├──     ThreadPool.cpp  
├── CMakeLists.txt        
├── build.sh              
└── run.sh     

```

---

🔧 关键组件说明
| 组件              | 功能描述                                                                                                                                                                                                                  |
| --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Server**      | - 初始化监听 TCP socket（非阻塞）并设置 socket 选项（端口复用、keep‑alive）<br>- 创建 epoll 实例并注册监听 socket<br>- 主循环中通过 `epoll_wait` 分发事件：<br>  - 新连接 → `accept4` → 注册客户端 fd<br>  - 可读/可写事件 → 派发给线程池处理<br>- 提供 `stop()` 接口，优雅关闭线程池、清理所有连接并释放资源 |
| **Connection**  | - 管理单个客户端的全生命周期，包括文件描述符、协议上下文和读写缓冲区<br>- 在可读时调用 `handleRead()`：从协议层循环读取完整包、业务处理、将响应加入发送队列<br>- 在可写时调用 `handleWrite()`：尽最大努力刷新发送缓冲区，写完后自动移除写事件<br>- 封装对 epoll 事件的动态增删（`EPOLLOUT`）                                      |
| **Protocol**    | - 底层数据帧管理：维护独立的接收缓冲区和发送缓冲区<br>- `tryReceivePacket()`：非阻塞地从接收缓冲区解析数据包，支持包未到达时返回「重试」或结束时返回「错误」<br>- `enqueuePacket()`：将业务层构造的包序列化并追加到发送缓冲区<br>- `flushSendBuffer()`：非阻塞地将发送缓冲区数据写入 socket，并报告错误或剩余情况                    |
| **Packet**      | - 定义数据包格式：<br>  1. **2 字节固定头**（标识协议）<br>  2. **2 字节长度字段**（payload 大小）<br>  3. **2 字节校验和**（简单加和）<br>  4. **可变长 payload**<br>- 提供 `serialize()`、`deserialize()`，实现字节序转换与校验和验证                                           |
| **ThreadPool**  | - 固定大小工作线程池：启动时创建 N 个 worker，支持异步任务提交<br>- 内部维护任务队列、互斥锁和条件变量，保证线程安全<br>- `enqueue()`：将可调用对象推入队列；<br>- `shutdown()` + `wait()`：停止接收新任务并等待所有线程退出                                                                        |
| **LoadTester**  | - 异步并发客户端压测工具，采用非阻塞 `connect` + epoll 事件驱动模式<br>- 在多线程环境下批量发起千万级连接请求<br>- 自动完成连接检测、发送单条消息、接收回显、统计成功/失败并打印吞吐                                                                                                           |
| **Server 入口**   | - 解析或设置默认端口、线程池大小、keep‑alive 等配置<br>- 注册信号处理（SIGINT/SIGTERM）以支持 Ctrl+C 优雅退出<br>- 调用 `setup()` 构建资源，随后 `run()` 进入阻塞事件循环                                                                                                |
| **Client Demo** | - 简单命令行交互式客户端，使用同一协议层发送任意字符串到服务端，并打印回显<br>- 示范 `Protocol`/`Packet` 在客户端的使用方法                                                                                                                                          |
---

## ⚙️ 环境与依赖

* **操作系统**：Linux（内核 ≥ 4.x，支持 epoll）
* **编译工具**：

  * CMake ≥ 3.10
  * 支持 C++20 的编译器（GCC ≥ 10 / Clang ≥ 11）
* **库 & API**：

  * POSIX sockets、`epoll`、`fcntl`、TCP keep‑alive
  * `<thread>`、`<mutex>`、`<condition_variable>`、`<atomic>`

---

## 🚀 快速上手

1. **克隆仓库**

   ```bash
   git clone https://github.com/yourname/MiniCommStack.git
   cd MiniCommStack
   ```

2. **调整系统参数**（可选，针对高并发）

   ```bash
   ulimit -n 2000000
   sudo sysctl -w net.ipv4.ip_local_port_range="10000 65000"
   sudo sysctl -w net.ipv4.tcp_tw_reuse=1
   ```

3. **一键构建**

   ```bash
   chmod +x build.sh run.sh
   ./build.sh
   ```

4. **启动并压测**

   ```bash
   ./run.sh
   ```

   或分步执行：

   ```bash
   # 启动服务器（后台）
   ./build/server &

   # 等待启动
   sleep 1

   # 压测：模拟 1000 连接，每连接 100 条消息
   ./build/load_test 127.0.0.1 8888 1000 100

   # 停止服务器
   kill %1
   ```

---

## 📊 性能测试示例

| 场景                   | 总消息     | 耗时     | 吞吐 (msg/s) | 成功率   |
| -------------------- | ------- | ------ | ---------- | ----- |
| 1 000 连接 × 100 条消息   | 100000  | 3.74 s | 26 729     | 100 % |
| 100 000 连接 × 1 条消息   | 100000  | 5.20 s | 19 200     | 100 % |
| 1 000 000 连接 × 1 条消息 | 1000000 | \~60 s | \~16 700   | 100 % |

> 实际结果会受机器配置、内核调优和网络状况影响，仅供参考。

---

## 🗺️ 开发计划

| 阶段       | 目标                                                | 预估时间  |
| -------- | ------------------------------------------------- | ----- |
| **阶段 1** | 单元测试 (`Protocol`/`Packet`)、`clang-format`、CI 集成   | 1–2 周 |
| **阶段 2** | 交叉编译（ARM/Yocto）、生成库文件、Docker 镜像                   | 2–3 周 |
| **阶段 3** | TLS 支持、流控 & 速率限制、防半开连接, DDoS 缓解                   | 3–4 周 |
| **阶段 4** | 性能优化（零拷贝、`io_uring`、多 Reactor/无锁设计）               | 4–6 周 |
| **阶段 5** | 文档 (Doxygen)、Python/C 客户端示例、博客“百万并发 Epoll Server” | 6–8 周 |

> 时间预估为个人业余项目，可根据实际情况调整。

---

## 🤝 贡献

* **提 Issue**：Bug 报告、功能建议
* **提交 PR**：优化代码、补充文档、增加测试

---

## 📜 许可证

本项目采用 **MIT License**，详见 [LICENSE](LICENSE)。
欢迎随意使用或学习，但请保留原作者声明。
