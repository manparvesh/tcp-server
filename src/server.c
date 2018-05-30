//
// Created by manparvesh on 30/5/18.
//

#include "server.h"

/**
 * creates a socket for the server and makes it passive such that
 * we can wait for connections on it later
 *
 * It uses `INADDR_ANY` (0.0.0.0) to bind to all the interfaces
 * available.
 *
 * PORT is defined in `server.h`
 * */
int server_listen(server_t *server) {
    int error = 0;
    struct sockaddr_in server_addr = {0};

    /**
     * `sockaddr_in` provides ways of representing a full address
     * composed of an IP address and port
     * */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = PORT;

    /**
     * The `socket(2)` syscall creates an endpoint for communication
     * and returns a file descriptor that refers to that endpoint.
     * */
    error = (server->listen_fd = socket(AF_INET, SOCK_STREAM, 0));
    if (error == -1) {
        perror("socket perror");
        printf("Failed to create socket endpoint\n");
        return error;
    }

    /**
     * bind() assigns the address specified to the socket referred to by
     * the file descriptor (`listen_fd`).
     * */
    error = bind(server->listen_fd,
                 (struct sockaddr *) &server_addr,
                 sizeof(server_addr));
    if (error == -1) {
        perror("bind");
        printf("Failed to bind socket to address\n");
        return error;
    }

    /**
     * listen() marks the socket referred to by sockfd as a passive socket
     * that is, as a socket that will be used to accept incoming connection
     * requests using accept(2).
     * */
    error = listen(server->listen_fd, BACKLOG);
    if (error == -1) {
        perror("listen");
        printf("Failed to put socket in passive mode\n");
        return error;
    }

    return 0;
}

/**
 * Accepts new connections and then prints `Hello World` to them
 * */
int server_accept(server_t *server) {
    // todo
}