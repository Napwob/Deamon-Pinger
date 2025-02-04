#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/un.h>
#include <signal.h>

#include "ping_stat.h"
#include "icmp_ping.h"

int unix_socket;
#define SOCKET_PATH "/tmp/usock"

static int create_unix_socket(char *sock_path)
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

static inline int parse_ping_command(const char *buffer, char *ip_addr, int *ping_count)
{
    if (sscanf(buffer, "ping %15s %d", ip_addr, ping_count) == 2)
    {
        return 0;
    }
    return 1;
}

static inline void free_and_exit()
{
    PingStat_free();
    unlink("/tmp/usock");
    close(unix_socket);
    exit(1);
}

static inline void remove_socket_if_exists()
{
    if (access(SOCKET_PATH, F_OK) == 0)
    {
        unlink(SOCKET_PATH);
    }
}

int main()
{
    remove_socket_if_exists();
    signal(SIGTERM, free_and_exit);
    signal(SIGINT, free_and_exit);

    PingData ping_data;

    // PingStat_print();

    unix_socket = create_unix_socket("/tmp/usock");
    if (unix_socket < 0)
        exit(1);

    listen(unix_socket, 5);

    while (1)
    {
        int client = accept(unix_socket, NULL, NULL);
        char buffer[256];

        int rec_len = read(client, buffer, sizeof(buffer));
        if (rec_len != -1)
            buffer[rec_len] = '\0';

        if (strncmp(buffer, "ping", sizeof("ping") - 1) == 0)
        {
            int ping_number;
            char ip_addr[16];
            if (parse_ping_command((const char *)&buffer, ip_addr, &ping_number) == 0)
            {
                ICMP_ping(ip_addr, ping_number, &ping_data);
                PingStat_update_s(&ping_data);
                write(client, "ok\n", 2);
            }
        }

        if (strncmp(buffer, "show", sizeof("show") - 1) == 0)
        {
            PingStat_socket_write(client);
        }
    }

    free_and_exit();
    return 0;
}