#ifndef COMMON_H
#define COMMON_H

#define DEFAULT_PORT 5500
#define BUF_SIZE 4096

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
    typedef int socklen_t;
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #define closesocket close
#endif

static int net_init(void) {
#ifdef _WIN32
    WSADATA wsa;
    return WSAStartup(MAKEWORD(2, 2), &wsa);
#else
    return 0;
#endif
}

static void net_cleanup(void) {
#ifdef _WIN32
    WSACleanup();
#endif
}

#endif
