//
// Created by manparvesh on 30/5/18.
//

#include "server.h"


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
        case NOTFOUND:
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

    if (type == ERROR || type == NOTFOUND || type == FORBIDDEN) {
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
        logger(NOTFOUND, "failed to open file", &buffer[5], fd);
    }
    logger(LOG, "SEND", &buffer[5], hit);
    len = (long) lseek(file_fd, (off_t) 0, SEEK_END); /* lseek to the file end to find the length */
    (void) lseek(file_fd, (off_t) 0, SEEK_SET); /* lseek back to the file start ready for reading */
    (void) sprintf(buffer,
                   "HTTP/1.1 200 OK\n"
                   "Server: nweb/%d.0\n"
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
