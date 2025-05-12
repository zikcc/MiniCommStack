cmake_minimum_required(VERSION 3.14)
project(VisionTests LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)
find_package(Threads REQUIRED)

# 测试可执行文件
add_executable(v4l2_tests
    test_v4l2.cpp
)

# 包含主工程的头文件
target_include_directories(v4l2_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../include
)

# 链接 vision 库、GTest、线程库
target_link_libraries(v4l2_tests
    PRIVATE
      vision
      GTest::GTest
      GTest::Main
      Threads::Threads
)

# 自动发现并注册测试
include(GoogleTest)
gtest_discover_tests(v4l2_tests)
