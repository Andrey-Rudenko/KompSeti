#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

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

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Использование: %s <ip_сервера> <порт> <путь_на_сервере> <куда_сохранить>\n", argv[0]);
        fprintf(stderr, "Пример: %s 127.0.0.1 5500 files/test.txt test.txt\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    int port = atoi(argv[2]);
    const char *remote_path = argv[3];
    const char *local_path = argv[4];

    net_init();

    int sock = (int)socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Ошибка создания сокета\n");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons((unsigned short)port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Не удалось подключиться к серверу\n");
        return 1;
    }

    char request[600];
    snprintf(request, sizeof(request), "GET %s\n", remote_path);
    send(sock, request, (int)strlen(request), 0);

    char header[64];
    if (read_line(sock, header, sizeof(header)) < 0) {
        fprintf(stderr, "Сервер не ответил\n");
        return 1;
    }

    long size = 0;
    if (sscanf(header, "OK %ld", &size) == 1) {
        FILE *f = fopen(local_path, "wb");
        if (!f) {
            fprintf(stderr, "Не удалось создать файл %s\n", local_path);
            return 1;
        }

        char buf[BUF_SIZE];
        long received = 0;
        while (received < size) {
            long left = size - received;
            int to_read = left < (long)sizeof(buf) ? (int)left : (int)sizeof(buf);
            int n = recv(sock, buf, to_read, 0);
            if (n <= 0) break;
            fwrite(buf, 1, n, f);
            received += n;
        }

        fclose(f);
        printf("Файл получен: %s (%ld из %ld байт) -> сохранён как %s\n", remote_path, received, size, local_path);
    } else {
        printf("Сервер ответил: %s\n", header);
    }

    closesocket(sock);
    net_cleanup();
    return 0;
}
