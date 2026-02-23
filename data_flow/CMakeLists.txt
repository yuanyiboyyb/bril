cmake_minimum_required(VERSION 3.10)
project(data_flow)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    src/main.cpp
    src/ir.cpp
    src/pass.cpp
)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()
# 添加源文件
add_executable(data_flow ${SOURCES})

# 添加头文件搜索路径
target_include_directories(data_flow PRIVATE ${CMAKE_SOURCE_DIR}/include)