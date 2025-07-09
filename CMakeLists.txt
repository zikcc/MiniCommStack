cmake_minimum_required(VERSION 3.10)
project(MiniCommStack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE Debug)  # 确保 Debug 模式

# 线程库
find_package(Threads REQUIRED)

# GoogleTest（嵌入式在 tests 子目录中引入）
add_subdirectory(thirdparty/googletest)

# 如果要编译测试
option(BUILD_TESTING "Build tests" ON)
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()


# 1) 协议
set(NET_SRCS
    src/net/Protocol.cpp
    src/net/Packet.cpp
)

# 2) 连接
set(CONNECTION_SRCS
    src/net/Connection.cpp
    src/net/ConnectionManager.cpp
)

# 3) threading 线程池
set(THREADING_SRCS
    src/threading/ThreadPool.cpp
)

# ----- server -----
add_executable(server
    main/main_server.cpp
    src/app/Server.cpp
    ${NET_SRCS}
    ${CONNECTION_SRCS}
    ${THREADING_SRCS}
)
target_include_directories(server PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(server PRIVATE
    Threads::Threads
)

# ----- client -----
add_executable(client
    main/main_client.cpp
    ${NET_SRCS}     
)
target_include_directories(client PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)

# ----- load_test -----
add_executable(load_test
    main/main_load_test.cpp
    src/load_test/LoadTester.cpp
    ${NET_SRCS}
)
target_include_directories(load_test PRIVATE
    ${PROJECT_SOURCE_DIR}/include
)
target_link_libraries(load_test PRIVATE
    Threads::Threads
)

