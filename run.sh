#!/bin/bash
set -e

# 启动服务器（后台运行）
echo "[*] Starting server..."
./build/server &

# 保存 PID 方便等会杀掉
SERVER_PID=$!

# 等待服务器启动稳定
sleep 1

# 运行压力测试
echo "[*] Running load test..."
./build/load_test 127.0.0.1 8888 1000 100

# 杀掉后台服务器
echo "[*] Stopping server..."
kill $SERVER_PID
wait $SERVER_PID 2>/dev/null || true