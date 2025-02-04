#ifndef PING_STAT_H
#define PING_STAT_H

#include <netinet/in.h>

typedef struct PingData
{
    struct in_addr addr;
    int sent;
    int received;
} PingData;

typedef struct PingStat
{
    PingData data;
    struct PingStat *next;
} PingStat;

void PingStat_add(const struct in_addr *addr, int sent, int received);
void PingStat_add_s(const PingData *stat);

void PingStat_update(const struct in_addr *addr, int sent, int received);
void PingStat_update_s(const PingData *stat);

void PingStat_socket_write(int fd);

void PingStat_print();
void PingStat_free();

#endif // PING_STAT_H
