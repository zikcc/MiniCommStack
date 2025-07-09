cmake_minimum_required(VERSION 3.14)
project(Tests LANGUAGES CXX)
# 自动发现并注册测试
include(GoogleTest)
set(CMAKE_CXX_STANDARD 17)

find_package(GTest REQUIRED)


