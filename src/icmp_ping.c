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

#include "icmp_ping.h"

#define BUFFER_SIZE 64

static unsigned short csum(unsigned short *addr, int len)
{
    int nleft = len;
    int sum = 0;
    unsigned short *w = addr;
    unsigned short answer = 0;

    while (nleft > 1)
    {
        sum += *w++;
        nleft -= 2;
    }

    if (nleft == 1)
    {
        *(unsigned char *)(&answer) = *(unsigned char *)w;
        sum += answer;
    }

    sum = (sum >> 16) + (sum & 0xFFFF);
    sum += (sum >> 16);
    answer = ~sum;

    return answer;
}

static inline int set_socket_timeout(int *s, int sec, int usec)
{
    struct timeval timeout;
    timeout.tv_sec = sec;
    timeout.tv_usec = usec;
    if (setsockopt(*s, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) < 0)
    {
        perror("setsockopt failed");
        close(*s);
        return 1;
    }

    return 0;
}

static void config_icmp_header(struct icmphdr *icmp_header)
{
    memset(icmp_header, 0, sizeof(*icmp_header));
    icmp_header->type = ICMP_ECHO;
    icmp_header->code = 0;
    icmp_header->un.echo.id = getpid() & 0xFFFF;
}

int ICMP_ping(const char *ip_addr, int ping_number, PingData *ping_data)
{
    if (ping_data == NULL)
    {
        fprintf(stderr, "PingData structure is NULL\n");
        return 1;
    }

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (s < 0)
    {
        perror("Error calling socket");
        return 1;
    }

    if (set_socket_timeout(&s, 1, 0))
    {
        perror("Error calling set_socket_timeout");
        close(s);
        return 1;
    }

    struct sockaddr_in sender_addr;
    sender_addr.sin_family = AF_INET;
    sender_addr.sin_addr.s_addr = inet_addr(ip_addr);

    struct icmphdr icmp_header;
    config_icmp_header(&icmp_header);

    int sent = 0;
    int recieved = 0;
    char buffer[BUFFER_SIZE];

    struct sockaddr_in reciever_addr;
    socklen_t reciever_len = sizeof(reciever_addr);

    for (int ping_counter = 0; ping_counter < ping_number; ping_counter++)
    {
        icmp_header.un.echo.sequence = htons(ping_counter + 1);

        icmp_header.checksum = 0;
        icmp_header.checksum = csum((unsigned short *)&icmp_header, sizeof(icmp_header));

        if (sendto(s, &icmp_header, sizeof(icmp_header), 0, (struct sockaddr *)&sender_addr, sizeof(sender_addr)) < 0)
        {
            perror("sendto() failed");
            continue;
        }
        sent++;

        memset(buffer, 0, sizeof(buffer));
        if (recvfrom(s, buffer, BUFFER_SIZE, 0, (struct sockaddr *)&reciever_addr, &reciever_len) < 0)
        {
            perror("recvfrom() failed");
            continue;
        }

        struct icmphdr *icmp_resp = (struct icmphdr *)(buffer + 20);
        if (icmp_resp->type == ICMP_ECHOREPLY)
        {
            recieved++;
        }
    }

    ping_data->addr = sender_addr.sin_addr;
    ping_data->received = recieved;
    ping_data->sent = sent;

    close(s);
    return 0;
}
