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
#include <pthread.h>

#include "ping_stat.h"
#include "icmp_ping.h"

#define SOCKET_PATH "/tmp/usock"

int unix_socket;

typedef struct
{
    char ip_addr[16];
    int ping_count;
} PingThreadArgs;

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

static int parse_ping_command(const char *buffer, char *ip_addr, int *ping_count)
{
    if (sscanf(buffer, "ping %15s %d", ip_addr, ping_count) == 2)
    {
        return 0;
    }
    return 1;
}

static void free_and_exit()
{
    PingStat_free();
    unlink(SOCKET_PATH);
    close(unix_socket);
    exit(1);
}

static void remove_socket_if_exists()
{
    if (access(SOCKET_PATH, F_OK) == 0)
    {
        unlink(SOCKET_PATH);
    }
}

static void *ping_worker(void *args)
{
    PingThreadArgs *ping_args = (PingThreadArgs *)args;

    PingData ping_data;
    memset(&ping_data, 0, sizeof(PingData));

    ICMP_ping(ping_args->ip_addr, ping_args->ping_count, &ping_data);
    PingStat_update_s(&ping_data);

    free(ping_args);
    return NULL;
}

static void daemonize()
{
    pid_t pid;

    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    umask(0);

    chdir("/");

    int x;
    for (x = sysconf(_SC_OPEN_MAX); x>=0; x--)
    {
        close (x);
    }
}

int main()
{
    daemonize();

    remove_socket_if_exists();
    signal(SIGTERM, free_and_exit);
    signal(SIGINT, free_and_exit);

    unix_socket = create_unix_socket(SOCKET_PATH);
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

            if (parse_ping_command(buffer, ip_addr, &ping_number) == 0)
            {
                PingThreadArgs *args = malloc(sizeof(PingThreadArgs));
                strcpy(args->ip_addr, ip_addr);
                args->ping_count = ping_number;

                pthread_t thread_id;
                pthread_create(&thread_id, NULL, ping_worker, args);
                pthread_detach(thread_id);

                write(client, "ok\n", 3);
            }
        }
        else if (strncmp(buffer, "show", sizeof("show") - 1) == 0)
        {
            PingStat_socket_write(client);
        }

        close(client);
    }

    free_and_exit();
    return 0;
}
