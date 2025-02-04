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

int create_unix_socket(char *sock_path)
{
    int unix_socket = socket(AF_UNIX, SOCK_STREAM, 0);
    if (unix_socket < 0)
    {
        perror("Error calling socket");
        return -1;
    }

    struct sockaddr_un addr;
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, sock_path);

    if (bind(unix_socket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        perror("Error calling bind");
        return -1;
    }

    return unix_socket;
}

int parse_ping_command(const char *buffer, char *ip_addr, int *ping_count)
{
    if (sscanf(buffer, "ping %15s %d", ip_addr, ping_count) == 2)
    {
        return 0;
    }
    return 1;
}

void *ping_worker(void *args)
{
    PingThreadArgs *ping_args = (PingThreadArgs *)args;

    PingData ping_data;
    memset(&ping_data, 0, sizeof(PingData));

    ICMP_ping(ping_args->ip_addr, ping_args->ping_count, &ping_data);
    PingStat_update_s(&ping_data);

    free(ping_args);
    return NULL;
}

void remove_socket_if_exists()
{
    if (access(SOCKET_PATH, F_OK) == 0)
    {
        unlink(SOCKET_PATH);
    }
}
