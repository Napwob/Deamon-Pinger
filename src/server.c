#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include "ping_stat.h"
#include "unix_socket.h"
#include "icmp_ping.h"

#include "server.h"

static void launch_ping_thread(const char *ip_addr, int ping_count, int client)
{
    PingThreadArgs *args = malloc(sizeof(PingThreadArgs));
    if (!args)
    {
        perror("malloc error");
        return;
    }

    snprintf(args->ip_addr, sizeof(args->ip_addr), "%s", ip_addr);
    args->ping_count = ping_count;

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, ping_worker, args) != 0)
    {
        perror("pthread_create error");
        free(args);
        return;
    }

    pthread_detach(thread_id);
    if (write(client, "ok\n", 3) < 0)
        perror("write error");
}

static int parse_ping_command(const char *buffer, char *ip_addr, int *ping_count)
{
    if (!buffer || !ip_addr || !ping_count)
        return -1;

    return (sscanf(buffer, "ping %15s %d", ip_addr, ping_count) == 2) ? 0 : -1;
}

static void handle_client_command(int client, char *buffer)
{
    if (strncmp(buffer, "ping", 4) == 0)
    {
        int ping_number;
        char ip_addr[16];
        if (parse_ping_command(buffer, ip_addr, &ping_number) == 0)
        {
            launch_ping_thread(ip_addr, ping_number, client);
        }
        else
        {
            if (write(client, "Invalid ping command\n", 21) < 0)
                perror("write error");
        }
    }
    else if (strncmp(buffer, "show", 4) == 0)
    {
        PingStat_socket_write(client);
    }
    else
    {
        if (write(client, "Unknown command\n", 17) < 0)
            perror("write error");
    }
}

int run_unix_socket_server(int unix_socket)
{
    while (1)
    {
        int client = accept(unix_socket, NULL, NULL);
        if (client < 0)
        {
            if (errno == EINTR)
                continue;
            perror("accept error");
            return -1;
        }

        char buffer[256] = {0};
        ssize_t rec_len = read(client, buffer, sizeof(buffer) - 1);
        if (rec_len < 0)
        {
            perror("read error");
            close(client);
            continue;
        }
        buffer[rec_len] = '\0';

        handle_client_command(client, buffer);
        close(client);
    }
    return 0;
}
