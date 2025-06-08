cmake_minimum_required(VERSION 3.10)
project(Lab2Test)

set(CMAKE_CXX_STANDARD 11)

# 添加 GoogleTest
add_subdirectory(googletest-main)

# 包含当前目录的头文件（main2.h）
include_directories(${PROJECT_SOURCE_DIR})

# 创建可执行测试目标
add_executable(runTests test_main2.cpp main2.cpp)

# 链接 GoogleTest 库
target_link_libraries(runTests gtest gtest_main)
