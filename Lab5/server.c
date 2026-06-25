#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define REQUEST_MAX 512

static int read_line(int sock, char *buf, int max_len) {
    int i = 0;
    while (i < max_len - 1) {
        char c;
        int n = recv(sock, &c, 1, 0);
        if (n <= 0) return -1;
        if (c == '\n') break;
        if (c != '\r') buf[i++] = c;
    }
    buf[i] = '\0';
    return i;
}

static void handle_client(int client_sock) {
    char request[REQUEST_MAX];
    if (read_line(client_sock, request, sizeof(request)) < 0) {
        return;
    }

    char cmd[16], path[REQUEST_MAX];
    if (sscanf(request, "%15s %500[^\n]", cmd, path) != 2 || strcmp(cmd, "GET") != 0) {
        const char *err = "ERR BAD_REQUEST\n";
        send(client_sock, err, (int)strlen(err), 0);
        return;
    }

    printf("Запрос файла: %s\n", path);

    FILE *f = fopen(path, "rb");
    if (!f) {
        const char *err = "ERR NOT_FOUND\n";
        send(client_sock, err, (int)strlen(err), 0);
        printf("Файл не найден: %s\n", path);
        return;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);

    char header[64];
    snprintf(header, sizeof(header), "OK %ld\n", size);
    send(client_sock, header, (int)strlen(header), 0);

    char buf[BUF_SIZE];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), f)) > 0) {
        send(client_sock, buf, (int)n, 0);
    }

    fclose(f);
    printf("Файл отправлен: %s (%ld байт)\n", path, size);
}

int main(int argc, char *argv[]) {
    int port = DEFAULT_PORT;
    if (argc > 1) port = atoi(argv[1]);

    net_init();

    int listen_sock = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (listen_sock < 0) {
        fprintf(stderr, "Ошибка создания сокета\n");
        return 1;
    }

    int opt = 1;
    setsockopt(listen_sock, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons((unsigned short)port);

    if (bind(listen_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Ошибка bind\n");
        return 1;
    }

    if (listen(listen_sock, 5) < 0) {
        fprintf(stderr, "Ошибка listen\n");
        return 1;
    }

    printf("TCP файловый сервер запущен на порту %d\n", port);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);
        int client_sock = (int)accept(listen_sock, (struct sockaddr *)&client_addr, &client_len);
        if (client_sock < 0) continue;

        printf("Подключился клиент %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

        handle_client(client_sock);
        closesocket(client_sock);
    }

    closesocket(listen_sock);
    net_cleanup();
    return 0;
}
