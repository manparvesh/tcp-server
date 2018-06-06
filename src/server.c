/*********************************************************
 *           Created by manparvesh on 30/5/18.
 *********************************************************/

#include "server.h"

int main(int argc, char **argv) {
    int i, port, pid, listenfd, socketfd, hit;
    socklen_t length;
    static struct sockaddr_in client_addr;
    static struct sockaddr_in server_addr;

    if (argc < 3 || argc > 3 || !strcmp(argv[1], "-?")) {
        printf("hint: Port-Number Top-Directory\t\tversion %d\n\n"
               "\ttcp_server is a small and very safe mini web server\n"
               "\tIt only servers out file/web pages with extensions named below\n"
               "\t and only from the named directory or its sub-directories.\n"
               "\tThere is no fancy features = safe and secure.\n\n"
               "\tExample: ./tcp_server 8181 /home/tcp_server_dir &\n\n"
               "\tOnly Supports:", VERSION);
        for (i = 0; extensions[i].extension != 0; i++) {
            printf(" %s", extensions[i].extension);
        }

        printf("\n\tNot Supported: URLs including \"..\", Java, Javascript, CGI\n"
               "\tNot Supported: directories / /etc /bin /lib /tmp /usr /dev /sbin \n"
               "\tNo warranty given or implied\n"
               "\tImplementation largely based on https://github.com/ankushagarwal/nweb\n");
        exit(0);
    }

    if (!strncmp(argv[2], "/", 2) || !strncmp(argv[2], "/etc", 5) ||
        !strncmp(argv[2], "/bin", 5) || !strncmp(argv[2], "/lib", 5) ||
        !strncmp(argv[2], "/tmp", 5) || !strncmp(argv[2], "/usr", 5) ||
        !strncmp(argv[2], "/dev", 5) || !strncmp(argv[2], "/sbin", 6)) {
        (void) printf("ERROR: Bad top directory %s, see tcp_server -?\n", argv[2]);
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
    logger(LOG, "tcp_server starting", argv[1], getpid());
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

/**
 * header file function implementations below:
 * */

void logger(enum log_type type, char *s1, char *s2, int socket_fd) {
    int fd;
    char log_buffer[BUFFER_SIZE * 2];

    switch (type) {
        case ERROR:
            sprintf(log_buffer, "ERROR: %s:%s Errorno=%d exiting pid=%d\n", s1, s2, errno, getpid());
            break;
        case FORBIDDEN:
            write(socket_fd,
                  "HTTP/1.1 403 Forbidden\n"
                  "Content-Length: 185\n"
                  "Connection: close\n"
                  "Content-Type: text/html\n"
                  "\n"
                  "<html><head>\n"
                  "<title>403 Forbidden</title>\n"
                  "</head><body>\n"
                  "<h1>Forbidden</h1>\n"
                  "The requested URL, file type or operation is not allowed on this simple static file webserver.\n"
                  "</body></html>\n",
                  271);
            sprintf(log_buffer, "FORBIDDEN: %s:%s", s1, s2);
            break;
        case NOT_FOUND:
            write(socket_fd,
                  "HTTP/1.1 404 Not Found\n"
                  "Content-Length: 136\n"
                  "Connection: close\n"
                  "Content-Type: text/html\n"
                  "\n"
                  "<html><head>\n"
                  "<title>404 Not Found</title>\n"
                  "</head><body>\n"
                  "<h1>Not Found</h1>\n"
                  "The requested URL was not found on this server.\n"
                  "</body></html>\n",
                  224);
            sprintf(log_buffer, "NOT FOUND: %s:%s", s1, s2);
            break;
        case LOG:
            sprintf(log_buffer, "INFO: %s:%s:%d", s1, s2, errno);
    }

    /**
     * Write message from buffer to log file
     * */
    if ((fd = open("server.log", O_CREAT | O_WRONLY | O_APPEND, 0644)) >= 0) {
        write(fd, log_buffer, strlen(log_buffer));
        write(fd, "\n", 1);
        close(fd);
    }

    if (type == ERROR || type == NOT_FOUND || type == FORBIDDEN) {
        exit(3);
    }
}

/**
 * child web server process so that we can exit on errorss
 * */
void web(int fd, int hit) {
    int j;
    int file_fd;
    int buffer_length;

    long i;
    long ret;
    long len;

    char *file_string;

    static char buffer[BUFFER_SIZE + 1]; // static, so initialized with zeros

    ret = read(fd, buffer, BUFFER_SIZE);
    if (ret == 0 || ret == -1) {
        // read failure, stop
        logger(FORBIDDEN, "Failed to read browser request", "", fd);
    }

    if (ret > 0 && ret < BUFFER_SIZE) {
        // valid return code
        buffer[ret] = 0; // terminate buffer
    } else {
        buffer[0] = 0;
    }

    for (i = 0; i < ret; i++) {
        // remove CF and LF characters
        if (buffer[i] == '\r' || buffer[i] == '\n') {
            buffer[i] = '*';
        }
    }

    logger(LOG, "request", buffer, hit);

    if (strncmp(buffer, "GET ", 4) && strncmp(buffer, "get ", 4)) {
        logger(FORBIDDEN, "Only simple GET operation supported", buffer, fd);
    }

    for (i = 4; i < BUFFER_SIZE; i++) {
        // null terminate after the second space to ignore extra stuff
        if (buffer[i] == ' ') {/* string is "GET URL " +lots of other stuff */
            buffer[i] = 0;
            break;
        }
    }

    // check for illegal parent directory use ..
    for (j = 0; j < i - 1; j++) {
        if (buffer[j] == '.' && buffer[j + 1] == '.') {
            logger(FORBIDDEN, "Parent directory (..) path names not supported", buffer, fd);
        }
    }

    if (!strncmp(&buffer[0], "GET /\0", 6) ||
        !strncmp(&buffer[0], "get /\0", 6)) /* convert no filename to index file */
        (void) strcpy(buffer, "GET /index.html");

    // work out the file type and check we support it
    buffer_length = strlen(buffer);
    file_string = (char *) 0;
    for (i = 0; extensions[i].extension != 0; i++) {
        len = strlen(extensions[i].extension);
        if (!strncmp(&buffer[buffer_length - len], extensions[i].extension, len)) {
            file_string = extensions[i].file_type;
            break;
        }
    }
    if (file_string == 0) {
        logger(FORBIDDEN, "File extension not supported!", buffer, fd);
    }

    if ((file_fd = open(&buffer[5], O_RDONLY)) == -1) {  /* open the file for reading */
        logger(NOT_FOUND, "failed to open file", &buffer[5], fd);
    }
    logger(LOG, "SEND", &buffer[5], hit);
    len = (long) lseek(file_fd, (off_t) 0, SEEK_END); /* lseek to the file end to find the length */
    (void) lseek(file_fd, (off_t) 0, SEEK_SET); /* lseek back to the file start ready for reading */
    (void) sprintf(buffer,
                   "HTTP/1.1 200 OK\n"
                   "Server: tcp_server/%d.0\n"
                   "Content-Length: %ld\n"
                   "Connection: close\n"
                   "Content-Type: %s\n\n",
                   VERSION,
                   len,
                   file_string); /* Header + a blank line */
    logger(LOG, "Header", buffer, hit);
    (void) write(fd, buffer, strlen(buffer));

    /* send file in 8KB block - last block may be smaller */
    while ((ret = read(file_fd, buffer, BUFFER_SIZE)) > 0) {
        (void) write(fd, buffer, ret);
    }

    sleep(1);  /* allow socket to drain before signalling the socket is closed */
    close(fd);
    exit(1);

}
