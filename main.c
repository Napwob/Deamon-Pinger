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

#include "ping_stat.h"
#include "icmp_ping.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        puts("Invlaid arguments count. \"sudo ./main <ip addr> <ping number>\"");
        return 1;
    }

    int ping_number = atoi(argv[2]);

    PingData ping_data;

    ICMP_ping(argv[1], ping_number, &ping_data);
    PingStat_update_s(&ping_data);

    PingStat_print();
    PingStat_free();
}