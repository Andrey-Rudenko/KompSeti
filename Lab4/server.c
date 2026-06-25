#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common.h"

#define DOMAINS_FILE "domains.txt"
#define MAX_RECORDS 100

typedef struct {
    char domain[128];
    char ip[64];
} Record;

static Record records[MAX_RECORDS];
static int record_count = 0;

static void load_domains(const char *filename) {
    FILE *f = fopen(filename, "r");
    if (!f) {
        fprintf(stderr, "Не удалось открыть файл %s\n", filename);
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), f) && record_count < MAX_RECORDS) {
        char domain[128], ip[64];
        if (sscanf(line, "%127s %63s", domain, ip) == 2) {
            strcpy(records[record_count].domain, domain);
            strcpy(records[record_count].ip, ip);
            record_count++;
        }
    }

    fclose(f);
    printf("Загружено доменов: %d\n", record_count);
}

static const char *find_ip(const char *domain) {
    for (int i = 0; i < record_count; i++) {
        if (strcmp(records[i].domain, domain) == 0) {
            return records[i].ip;
        }
    }
    return NULL;
}

int main(void) {
    net_init();
    load_domains(DOMAINS_FILE);

    int sock = (int)socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        fprintf(stderr, "Ошибка создания сокета\n");
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Ошибка bind\n");
        return 1;
    }

    printf("UDP DNS-сервер запущен на порту %d\n", PORT);

    char buf[BUF_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    while (1) {
        memset(buf, 0, sizeof(buf));
        int n = recvfrom(sock, buf, sizeof(buf) - 1, 0, (struct sockaddr *)&client_addr, &client_len);
        if (n <= 0) continue;

        buf[n] = '\0';
        buf[strcspn(buf, "\r\n")] = '\0';

        printf("Запрос от %s:%d -> домен \"%s\"\n",
               inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), buf);

        const char *ip = find_ip(buf);
        const char *response = ip ? ip : "NOT_FOUND";

        sendto(sock, response, (int)strlen(response), 0, (struct sockaddr *)&client_addr, client_len);
    }

    closesocket(sock);
    net_cleanup();
    return 0;
}
