cmake_minimum_required(VERSION 3.30)
project(network_programming)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_executable(showip showip.cpp)

add_subdirectory(stream)
add_subdirectory(chat_room)