#include <stdio.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <sys/socket.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <arpa/inet.h>

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

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        puts("Invlaid arguments count. \"sudo ./main <ip addr> <ping number>\"");
        return 1;
    }

    int ping_number = atoi(argv[2]);

    int s = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (s < 0)
    {
        perror("Error calling socket");
        return 0;
    }

    struct sockaddr_in sender_addr;
    sender_addr.sin_family = AF_INET;
    sender_addr.sin_addr.s_addr = inet_addr(argv[1]);

    struct icmphdr icmp_header;
    memset(&icmp_header, 0, sizeof(icmp_header));

    icmp_header.type = ICMP_ECHO;
    icmp_header.code = 0;
    icmp_header.un.echo.id = getpid() & 0xFFFF;
    icmp_header.un.echo.sequence = 1;
    icmp_header.checksum = csum((unsigned short *)&icmp_header, sizeof(icmp_header));

    char buffer[1024];
    struct sockaddr_in reciever_addr;
    socklen_t reciever_len = sizeof(reciever_addr);

    int echoreply_count = 0;
    int error_count = 0;

    for (int ping_counter = 0; ping_counter < ping_number; ping_counter++)
    {
        if (sendto(s, &icmp_header, sizeof(icmp_header), 0, (struct sockaddr *)&sender_addr, sizeof(sender_addr)) < 0)
        {
            perror("sendto()");
            exit(3);
        }

        if (recvfrom(s, &buffer, 1024, 0, (struct sockaddr *)&reciever_addr, &reciever_len) < 0)
        {
            perror("recvfrom()");
            exit(3);
        }

        struct icmphdr *icmp_resp = (struct icmphdr *)(buffer + 20);
        if (icmp_resp->type == ICMP_ECHOREPLY)
        {
            echoreply_count++;
        }
        else
        {
            error_count++;
        }
    }

    printf("IP: %s %d/%d\n", argv[1], echoreply_count, error_count);
    //printf("Answer from %s: ICMP sequence=%d\n", inet_ntoa(reciever_addr.sin_addr), icmp_resp->un.echo.sequence);

    close(s);
}