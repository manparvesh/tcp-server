/*********************************************************
 *           Created by manparvesh on 30/5/18.
 *********************************************************/

#ifndef TCP_SERVER_SERVER_H
#define TCP_SERVER_SERVER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define VERSION 1
#define BUFFER_SIZE 8096

#ifndef SIGCLD
#   define SIGCLD SIGCHLD
#endif

enum log_type {
    ERROR = 42,
    LOG = 44,
    FORBIDDEN = 403,
    NOT_FOUND = 404
};

struct {
    char *extension;
    char *file_type;
} extensions[] = {
        {"gif",  "image/gif"},
        {"jpg",  "image/jpg"},
        {"jpeg", "image/jpeg"},
        {"png",  "image/png"},
        {"ico",  "image/ico"},
        {"zip",  "image/zip"},
        {"gz",   "image/gz"},
        {"tar",  "image/tar"},
        {"htm",  "text/html"},
        {"html", "text/html"},
        {0,      0}};

void logger(enum log_type type, char *s1, char *s2, int socket_fd);

/**
 * child web server process so that we can exit on errorss
 * */
void web(int fd, int hit);

#endif //TCP_SERVER_SERVER_H
