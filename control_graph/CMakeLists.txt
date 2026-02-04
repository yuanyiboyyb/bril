cmake_minimum_required(VERSION 3.10)
project(ControlGraph)

set(CMAKE_CXX_STANDARD 17)

# 添加源文件
add_executable(control_graph src/main.cpp)

# 添加头文件搜索路径
target_include_directories(control_graph PRIVATE ${CMAKE_SOURCE_DIR}/include)