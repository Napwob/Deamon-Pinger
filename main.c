#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "ping_stat.h"
#include "icmp_ping.h"
#include "deamon.h"
#include "unix_socket.h"

#define SOCKET_PATH "/tmp/usock"

int main()
{
    if (check_and_create_pid_file() < 0)
    {
        exit(1);
    }

    daemonize();

    remove_socket_if_exists();

    unix_socket = create_unix_socket(SOCKET_PATH);
    if (unix_socket < 0)
    {
        exit(1);
    }

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
