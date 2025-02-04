#include "ping_stat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

static PingStat *head = NULL;

void add_stat(struct in_addr addr, int sent, int received)
{
    PingStat *new_entry = (PingStat *)malloc(sizeof(PingStat));
    if (!new_entry)
    {
        perror("Memory allocation error");
        exit(1);
    }
    new_entry->data.addr = addr;
    new_entry->data.sent = sent;
    new_entry->data.received = received;
    new_entry->next = head;
    head = new_entry;
}

void add_stat_s(PingData stat)
{
    add_stat(stat.addr, stat.sent, stat.received);
}

void update_stat(struct in_addr addr, int sent, int received)
{
    PingStat *current = head;
    while (current)
    {
        if (memcmp(&current->data.addr, &addr, sizeof(struct in_addr)) == 0)
        {
            current->data.sent += sent;
            current->data.received += received;
            return;
        }
        current = current->next;
    }

    add_stat(addr, sent, received);
}

void update_stat_s(PingData stat)
{
    update_stat(stat.addr, stat.sent, stat.received);
}

void print_stats()
{
    PingStat *current = head;
    while (current)
    {
        printf("%s: sent %d, recieved %d\n", inet_ntoa(current->data.addr), 
                                             current->data.sent, 
                                             current->data.received);
        current = current->next;
    }
}

void free_stats()
{
    PingStat *current = head;
    while (current)
    {
        PingStat *temp = current;
        current = current->next;
        free(temp);
    }
}
