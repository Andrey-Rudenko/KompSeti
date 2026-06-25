#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Использование: %s <ip_сервера> <домен>\n", argv[0]);
        fprintf(stderr, "Пример: %s 127.0.0.1 yandex.ru\n", argv[0]);
        return 1;
    }

    const char *server_ip = argv[1];
    const char *domain = argv[2];

    net_init();

    int sock = (int)socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Ошибка создания сокета\n");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);

    sendto(sock, domain, (int)strlen(domain), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));

    char buf[BUF_SIZE];
    memset(buf, 0, sizeof(buf));
    struct sockaddr_in from_addr;
    socklen_t from_len = sizeof(from_addr);

    int n = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&from_addr, &from_len);
    if (n > 0) {
        buf[n] = '\0';
        printf("%s -> %s\n", domain, buf);
    } else {
        printf("Ответ не получен\n");
    }

    closesocket(sock);
    net_cleanup();
    return 0;
}
