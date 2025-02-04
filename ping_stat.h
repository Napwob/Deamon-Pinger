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

void add_stat(struct in_addr addr, int sent, int received);
void update_stat(struct in_addr addr, int sent, int received);
void add_stat_s(PingData stat);
void update_stat_s(PingData stat);
void print_stats();
void free_stats();

#endif // PING_STAT_H
