#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>
#include "ping_stat.h"
#include "icmp_ping.h"
#include "deamon.h"
#include "unix_socket.h"
#include "server.h"

#define SOCKET_PATH "/tmp/usock"

static int handle_arguments(int argc, char *argv[]);
static inline void show_help(void);

int main(int argc, char *argv[])
{
    switch (handle_arguments(argc, argv))
    {
    case -1:
        exit(EXIT_SUCCESS);
        break;
    case 0:
        Deamon_start();
        break;
    case 1:
        break;
    default:
        exit(EXIT_FAILURE);
    }

    if (Deamon_create_pid_file() < 0)
    {
        exit(EXIT_FAILURE);
    }

    UnixSocket_remove_if_exists();

    int unix_socket = UnixSocket_create(SOCKET_PATH);
    if (unix_socket < 0)
    {
        perror("Error creating UNIX socket");
        exit(EXIT_FAILURE);
    }

    Server_run(unix_socket);

    free_and_exit();
    return EXIT_SUCCESS;
}

static inline void show_help(void)
{
    puts("Available commands:");
    puts("  ping <ip_addr> <ping_count>   - start a ping to the specified IP address");
    puts("  show                          - display ping statistics");
    puts("Examples:");
    puts("  ping command: \"echo \"ping <ip_addr> <ping_count>\" | nc -U /tmp/usock\"");
    puts("  show command: \"echo \"show\" | nc -U /tmp/usock\"");
    puts("Arguments:");
    puts("  --debug  : run in debug mode");
    puts("  --help   : show help");
}

static int handle_arguments(int argc, char *argv[])
{
    if (argc == 2)
    {
        if (strncmp(argv[1], "--debug", 7) == 0)
        {
            puts("Debug mode");
            return 1;
        }
        else if (strncmp(argv[1], "--help", 6) == 0)
        {
            show_help();
            return -1;
        }
    }
    else if (argc > 2)
    {
        puts("Invalid arguments");
        show_help();
        return -1;
    }

    return 0;
}