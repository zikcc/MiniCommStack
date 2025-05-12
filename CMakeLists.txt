cmake_minimum_required(VERSION 3.10)
project(MiniCommStack LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# —— 找到 libv4l2 —— 
find_path(V4L2_INCLUDE_DIR
  NAMES linux/videodev2.h
  HINTS /usr/include /usr/local/include
)
find_library(V4L2_LIBRARY
  NAMES v4l2
  HINTS /usr/lib /usr/local/lib
)

# 线程库
find_package(Threads REQUIRED)

# OpenCV
set(OpenCV_DIR "/usr/local/lib/cmake/opencv4") 
find_package(OpenCV 4.9 REQUIRED)
message(STATUS "Found OpenCV ${OpenCV_VERSION}")

# GoogleTest（嵌入式在 tests 子目录中引入）
add_subdirectory(thirdparty/googletest)

# 如果要编译测试
option(BUILD_TESTING "Build tests" ON)
if (BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()

# —— vision 库 —— 
add_library(vision
    src/vision/V4L2Capture.cpp
    include/vision/V4L2Capture.hpp
)

# 导出头文件位置
target_include_directories(vision
    PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# 链接 libv4l2、OpenCV、线程库等
target_include_directories(vision
    PRIVATE ${V4L2_INCLUDE_DIR}
)
target_link_libraries(vision
    PRIVATE
      ${V4L2_LIBRARY}
      ${OpenCV_LIBS}
      Threads::Threads
)

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
