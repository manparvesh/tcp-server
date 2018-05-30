//
// Created by manparvesh on 30/5/18.
//

#ifndef TCP_SERVER_SERVER_H
#define TCP_SERVER_SERVER_H

#include <arpa/inet.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/epoll.h>
#include <sys/socket.h>

#define PORT 8080
#define BACKLOG 128

/**
 * Encapsulates the properties of the server
 * */
typedef struct server {
    /**
     * File descriptor of the socket in passive mode to wait for connections
     * */
    int listen_fd;
} server_t;

/**
 * creates a socket for the server and makes it passive such that we can wait for connections on it later
 * */
int server_listen(server_t *server);

/**
 * Accepts new connections and then prints `Hello World` to them
 * */
int server_accept(server_t *server);

#endif //TCP_SERVER_SERVER_H
