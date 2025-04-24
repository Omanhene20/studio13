#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCKET_PATH "/tmp/my_socket"
#define BUFFER_SIZE 256

int main() {
    int sock;
    struct sockaddr_un addr;
    char message[BUFFER_SIZE];
    char response[BUFFER_SIZE];

    sock = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    memset(&addr, 0, sizeof(struct sockaddr_un));
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_PATH, sizeof(addr.sun_path) - 1);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_un)) == -1) {
        perror("connect");
        exit(EXIT_FAILURE);
    }

    printf("Enter request (filename, ls, or quit): ");
    fgets(message, BUFFER_SIZE, stdin);
    write(sock, message, strlen(message));

    ssize_t num_read;
    while ((num_read = read(sock, response, BUFFER_SIZE - 1)) > 0) {
        response[num_read] = '\0';
        printf("%s", response);
    }

    close(sock);
    return 0;
}
