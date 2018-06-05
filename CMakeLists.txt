cmake_minimum_required(VERSION 3.10)
project(tcp_server)

set(CMAKE_CXX_STANDARD 11)

#add_executable(tcp_server
#        src/server.c)

add_executable(tcp_server_runner
        src/main.c src/server.c)
