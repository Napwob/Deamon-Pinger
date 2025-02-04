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

#include "ping_stat.h"
#include "icmp_ping.h"

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

int main(int argc, char *argv[])
{
    /*
        if (argc != 3)
        {
            puts("Invlaid arguments count. \"sudo ./main <ip addr> <ping number>\"");
            return 1;
        }

        int ping_number = atoi(argv[2]);
    */
    int unix_socket = create_unix_socket("/tmp/usock");
    if (unix_socket < 0)
        return -1;

    listen(unix_socket, 1);
    int client = accept(unix_socket, NULL, NULL);
    char buffer[256];

    read(client, buffer, sizeof(buffer));

    write(client, "ok", 2);

    unlink("/tmp/usock");
    /*
        PingData ping_data;

        ICMP_ping(argv[1], ping_number, &ping_data);
        PingStat_update_s(&ping_data);

        PingStat_print();
        PingStat_free();
    */
    return 0;
}