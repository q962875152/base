cmake_minimum_required(VERSION 3.20.0)

project(base)

set(CMAKE_CXX_STANDARD 20)  # 设置为C++17版本
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)  # 推荐: 确保 -std=c++17 而不是 -std=gnu++17

set(CMAKE_BUILD_TYPE Debug)

# 设置C编译器为Clang
set(CMAKE_C_COMPILER "clang")

# 设置C++编译器为Clang
set(CMAKE_CXX_COMPILER "clang++")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra")

# 设置bin是调试版本
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

SET(BIN_OUTPUT "base")

SET(PROJECT_ROOT_PATH "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(
    ${PROJECT_ROOT_PATH}/MWUtils
    ${PROJECT_ROOT_PATH}/src
    ${PROJECT_ROOT_PATH}/include
)

aux_source_directory(character BIN_SRC_FILES)
aux_source_directory(src BIN_SRC_FILES)

add_executable(${BIN_OUTPUT} ${BIN_SRC_FILES})