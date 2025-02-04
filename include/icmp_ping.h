#ifndef ICMP_PING_H
#define ICMP_PING_H

#include <netinet/in.h>

#include "ping_stat.h"

int ICMP_ping(const char *ip_addr, int ping_number, PingData *ping_data);

#endif // ICMP_PING_H
