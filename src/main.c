//
// Created by manparvesh on 30/5/18.
//

#include <stdio.h>
#include "server.h"


int main(int argc, char **argv) {
    int i, port, pid, listenfd, socketfd, hit;
    socklen_t length;
    static struct sockaddr_in client_addr;
    static struct sockaddr_in server_addr;

    if (argc < 3 || argc > 3 || !strcmp(argv[1], "-?")) {
        printf("hint: nweb Port-Number Top-Directory\t\tversion %d\n\n"
               "\tnweb is a small and very safe mini web server\n"
               "\tnweb only servers out file/web pages with extensions named below\n"
               "\t and only from the named directory or its sub-directories.\n"
               "\tThere is no fancy features = safe and secure.\n\n"
               "\tExample: nweb 8181 /home/nwebdir &\n\n"
               "\tOnly Supports:", VERSION);
        for (i = 0; extensions[i].extension != 0; i++) {
            printf(" %s", extensions[i].extension);
        }

        printf("\n\tNot Supported: URLs including \"..\", Java, Javascript, CGI\n"
               "\tNot Supported: directories / /etc /bin /lib /tmp /usr /dev /sbin \n"
               "\tNo warranty given or implied\n"
               "\tNigel Griffiths nag@uk.ibm.com\n");
        exit(0);
    }

    if (!strncmp(argv[2], "/", 2) || !strncmp(argv[2], "/etc", 5) ||
        !strncmp(argv[2], "/bin", 5) || !strncmp(argv[2], "/lib", 5) ||
        !strncmp(argv[2], "/tmp", 5) || !strncmp(argv[2], "/usr", 5) ||
        !strncmp(argv[2], "/dev", 5) || !strncmp(argv[2], "/sbin", 6)) {
        (void) printf("ERROR: Bad top directory %s, see nweb -?\n", argv[2]);
        exit(3);
    }
    if (chdir(argv[2]) == -1) {
        (void) printf("ERROR: Can't Change to directory %s\n", argv[2]);
        exit(4);
    }

    /* Become deamon + unstopable and no zombies children (= no wait()) */
    if (fork() != 0)
        return 0; /* parent returns OK to shell */

    (void) signal(SIGCLD, SIG_IGN); /* ignore child death */
    (void) signal(SIGHUP, SIG_IGN); /* ignore terminal hangups */
    for (i = 0; i < 32; i++)
        (void) close(i);    /* close open files */

    (void) setpgrp();    /* break away from process group */
    logger(LOG, "nweb starting", argv[1], getpid());
    /* setup the network socket */
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        logger(ERROR, "system call", "socket", 0);
    port = atoi(argv[1]);
    if (port < 0 || port > 60000)
        logger(ERROR, "Invalid port number (try 1->60000)", argv[1], 0);
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(port);
    if (bind(listenfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        logger(ERROR, "system call", "bind", 0);
    if (listen(listenfd, 64) < 0)
        logger(ERROR, "system call", "listen", 0);
    for (hit = 1;; hit++) {
        length = sizeof(client_addr);
        if ((socketfd = accept(listenfd, (struct sockaddr *) &client_addr, &length)) < 0)
            logger(ERROR, "system call", "accept", 0);
        if ((pid = fork()) < 0) {
            logger(ERROR, "system call", "fork", 0);
        } else {
            if (pid == 0) {   /* child */
                (void) close(listenfd);
                web(socketfd, hit); /* never returns */
            } else {   /* parent */
                (void) close(socketfd);
            }
        }
    }

    return 0;
}
