#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>
#include "ping_stat.h"
#include "icmp_ping.h"
#include "unix_socket.h"

#define SOCKET_PATH "/tmp/usock"

int unix_socket;

int create_unix_socket(const char *sock_path)
{
    if (!sock_path)
        return -1;

    int unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unix_socket < 0)
    {
        perror("Error calling socket");
        return -1;
    }

    struct sockaddr_un addr = {0};
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, sock_path, sizeof(addr.sun_path) - 1);

    if (bind(unix_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Error calling bind");
        close(unix_socket);
        unlink(sock_path);
        return -1;
    }

    if (listen(unix_socket, 5) < 0)
    {
        perror("Error calling listen");
        close(unix_socket);
        unlink(sock_path);
        return -1;
    }

    return unix_socket;
}

int parse_ping_command(const char *buffer, char *ip_addr, int *ping_count)
{
    if (!buffer || !ip_addr || !ping_count)
        return -1;

    return (sscanf(buffer, "ping %15s %d", ip_addr, ping_count) == 2) ? 0 : -1;
}

void *ping_worker(void *args)
{   
    if (!args)
        return NULL;

    PingThreadArgs *ping_args = (PingThreadArgs *)args;
    if (!ping_args->ip_addr || ping_args->ping_count <= 0)
        return NULL;

    PingData ping_data = {0};

    ICMP_ping(ping_args->ip_addr, ping_args->ping_count, &ping_data);
    PingStat_update_s(&ping_data);

    free(ping_args);
    ping_args = NULL;

    return NULL;
}

void remove_socket_if_exists()
{
    if (access(SOCKET_PATH, F_OK) == 0)
    {
        unlink(SOCKET_PATH);
    }
}
