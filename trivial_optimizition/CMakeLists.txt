cmake_minimum_required(VERSION 3.10)
project(trivial_opt)

set(CMAKE_CXX_STANDARD 17)

set(SOURCES
    src/main.cpp
    src/ir.cpp
)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Choose the type of build" FORCE)
endif()
# 添加源文件
add_executable(trivial_opt ${SOURCES})

# 添加头文件搜索路径
target_include_directories(trivial_opt PRIVATE ${CMAKE_SOURCE_DIR}/include)