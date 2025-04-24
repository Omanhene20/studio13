#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define SOCKET_PATH "/tmp/my_socket"
#define BUFFER_SIZE 256

void send_file(FILE *fp, int client_sock) {
    char file_buf[BUFFER_SIZE];
    while (fgets(file_buf, sizeof(file_buf), fp)) {
        write(client_sock, file_buf, strlen(file_buf));
    }
}

int main() {
    int server_sock, client_sock;
    struct sockaddr_un addr;
    char buffer[BUFFER_SIZE];

    server_sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (server_sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);
    unlink(SOCKET_PATH);

    if (bind(server_sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(server_sock, 5) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server is listening...\n");

    while (1) {
        client_sock = accept(server_sock, NULL, NULL);
        if (client_sock == -1) {
            perror("accept");
            continue;
        }

        ssize_t num_read = read(client_sock, buffer, BUFFER_SIZE - 1);
        if (num_read <= 0) {
            close(client_sock);
            continue;
        }

        buffer[num_read] = '\0';
        buffer[strcspn(buffer, "\n")] = '\0'; // Strip newline

        printf("Received request: %s\n", buffer);

        if (strncmp(buffer, "quit", 4) == 0) {
            printf("Shutdown signal received.\n");
            close(client_sock);
            break;
        } else if (strncmp(buffer, "ls", 2) == 0) {
            FILE *fp = popen("ls", "r");
            if (fp == NULL) {
                char *error_msg = "Failed to execute ls command.\n";
                write(client_sock, error_msg, strlen(error_msg));
            } else {
                send_file(fp, client_sock);
                pclose(fp);
            }
        } else {
            FILE *fp = fopen(buffer, "r");
            if (fp == NULL) {
                char *error_msg = "File not found or could not be opened.\n";
                write(client_sock, error_msg, strlen(error_msg));
            } else {
                send_file(fp, client_sock);
                fclose(fp);
            }
        }

        shutdown(client_sock, SHUT_RDWR);
        close(client_sock);
    }

    close(server_sock);
    unlink(SOCKET_PATH);
    return 0;
}
