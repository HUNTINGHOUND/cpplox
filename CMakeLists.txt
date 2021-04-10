cmake_minimum_required(VERSION 3.10)

project(cpplox VERSION 0.5)

set(CMAKE_CXX_STANDARD 17)

configure_file("config.h.in" ${PROJECT_BINARY_DIR}/config.h)
include_directories(cpplox PUBLIC ${PROJECT_BINARY_DIR})

add_executable(cpplox
cpplox/chunk.cpp
cpplox/compiler.cpp
cpplox/debug.cpp
cpplox/main.cpp
cpplox/memory.cpp
cpplox/object.cpp
cpplox/scanner.cpp
cpplox/value.cpp
cpplox/vm.cpp
cpplox/table.cpp)

