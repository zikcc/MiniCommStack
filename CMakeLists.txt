cmake_minimum_required(VERSION 3.10)
project(MiniCommStack)

# 设置C++标准
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
# 查找 curl
find_package(CURL REQUIRED)
# 添加include目录
include_directories(${PROJECT_SOURCE_DIR}/include)
# aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC_FILES)

# 添加可执行文件
# add_executable(server ${SRC_FILES})
# add_executable(client ${SRC_FILES})
add_executable(server src/Server.cpp src/Protocol.cpp src/Packet.cpp src/Crawler.cpp)
add_executable(client src/Client.cpp src/Protocol.cpp src/Packet.cpp)

# 链接 curl 到你的目标
target_link_libraries(server CURL::libcurl)