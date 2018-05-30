//
// Created by manparvesh on 30/5/18.
//

#include <stdio.h>
#include "server.h"


int main() {
    int error = 0;
    server_t server = {0};

    error = server_listen(&server);
    if (error) {
        printf("Failed to listen on address 0.0.0.0:%d\n", PORT);
        return error;
    }

    while (1) {
        error = server_accept(&server);
        if (error) {
            printf("Failed accepting connection\n");
            return error;
        }
    }

    return 0;
}
