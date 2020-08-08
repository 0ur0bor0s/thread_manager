cmake_minimum_required(VERSION 3.17)
project(thread_manager C)

set(CMAKE_C_STANDARD 99)

set(CMAKE_C_FLAGS -pthread)

add_executable(thread_manager thread_manager.c command_node.c command_node.h)