#pragma once
#include <cstddef>  // for size_t

struct ServerConfig {
    // 网络配置
    int port = 8888;
    int max_connections = 10000;
    int backlog = 1024;
    
    // 线程池配置
    int thread_pool_size = 4;  // 默认使用CPU核心数
    
    // 缓冲区配置
    size_t read_buffer_size = 8192;
    size_t write_buffer_size = 8192;
    
    // 超时配置（毫秒）
    int connection_timeout = 30000;  // 连接超时
    int read_timeout = 30000;        // 读取超时
    int write_timeout = 30000;       // 写入超时
    
    // 性能调优
    bool reuse_addr = true;          // SO_REUSEADDR
    bool reuse_port = true;          // SO_REUSEPORT
    bool keep_alive = true;          // SO_KEEPALIVE
    int keep_alive_time = 60;        // TCP keepalive time
    int keep_alive_intvl = 5;        // TCP keepalive interval
    int keep_alive_probes = 3;       // TCP keepalive probes
}; 