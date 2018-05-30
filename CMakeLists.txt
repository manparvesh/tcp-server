cmake_minimum_required(VERSION 3.10)
project(tcp_server)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(tcp_server
        src/server.c src/server.h src/main.c)
